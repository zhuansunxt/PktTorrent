/**
 * @file location_handler.c
 * @brief implementation for location handler.
 * @author Xiaotong Sun <xiaotons@andrew.cmu.edu>
 * @author Longqi Cai   <longqic@andrew.cmu.edu>
 */

#include <arpa/inet.h>
#include "location_handler.h"
#include "../utilities/spiffy.h"
#include "../utilities/chunk.h"
#include "../utilities/sha.h"

void ask_peers_who_has(g_state_t *g, session_nlchunk_t *nl_chunks) {
  packet_t *who_has_packet = build_who_has_packet(nl_chunks);
  broadcast_who_has_packets(g, who_has_packet);
  pkt_free(who_has_packet);
  return ;
}

packet_t* build_who_has_packet(session_nlchunk_t * nl_chunks) {
  unsigned int packet_len = 0;
  packet_t *who_has_packet = pkt_new();
  who_has_packet->hdr->magic = (uint16_t)htons(15441);    // hardcoded
  who_has_packet->hdr->version = 1;                       // hardcoded
  who_has_packet->hdr->type = 0;                          // WHOHAS
  who_has_packet->hdr->hlen = (uint16_t)htons(HDRSZ);     // fixed
  who_has_packet->hdr->seqn = (uint32_t)htonl(0);         // invalid
  who_has_packet->hdr->ackn = (uint32_t)htonl(0);         // invalid

  session_nlchunk_t *iter;
  uint8_t nlchunk_count = 0;
  for (iter = nl_chunks; iter; iter = iter->next) {
    nlchunk_count++;
  }

  /* number of chunk hashes. */
  memcpy(who_has_packet->payload, &nlchunk_count, sizeof(uint8_t));

  /* paddling for 32-bit alignment */
  memset(who_has_packet->payload+sizeof(uint8_t), 0, sizeof(uint8_t)*3);

  /* packet length */
  packet_len = HDRSZ + 4*sizeof(uint8_t) + nlchunk_count*(SHA1_HASH_SIZE);
  who_has_packet->hdr->plen = (uint16_t)htons(packet_len);

  /* copy chunk hashes */
  char *payload_ptr = (char*)who_has_packet->payload + 4*sizeof(uint8_t);
  for (iter = nl_chunks; iter; iter = iter->next) {
    char hash_hex[SHA1_HASH_SIZE];
    ascii2hex(iter->chunk_hash, SHA1_HASH_SIZE*2, (uint8_t*)hash_hex);
    memcpy(payload_ptr, hash_hex, SHA1_HASH_SIZE);
    payload_ptr += SHA1_HASH_SIZE;
  }

  return who_has_packet;
}

void broadcast_who_has_packets(g_state_t *g, packet_t* wh_packet) {
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  assert(fd != 0);

  bt_peer_t *peer_iter;

  for (peer_iter = g->g_config->peers; peer_iter; peer_iter = peer_iter->next) {
    if (peer_iter->id == g->g_config->identity) continue;
    console_log("Sent packet length: %u", ntohs(wh_packet->hdr->plen));
    send_packet(peer_iter->id, wh_packet, g);
#ifdef DEBUG
    console_log("Peer %d: Sent WHOHAS packet to peer %d", g->g_config->identity, peer_iter->id);
#endif
  }
}

void process_who_has_packet(g_state_t *g, packet_t* wh_packet, short id) {
  uint8_t num_of_chunks;
  memcpy(&num_of_chunks, wh_packet->payload, sizeof(uint8_t));

  if (num_of_chunks <= 0) return;

  packet_t * ihave_packet = pkt_new();
  ihave_packet->hdr->magic = (uint16_t)htons(15441);
  ihave_packet->hdr->version = 1;
  ihave_packet->hdr->type = 1;
  ihave_packet->hdr->hlen = (uint16_t)htons(HDRSZ);
  ihave_packet->hdr->seqn = (uint32_t)htonl(0);
  ihave_packet->hdr->ackn = (uint32_t)htonl(0);

  uint8_t i = 0;
  uint8_t *chunk_ptr = wh_packet->payload + 4*sizeof(uint8_t);
  char *ihave_payload_ptr = (char*)ihave_packet->payload + 4*sizeof(uint8_t);
  uint8_t ihave_chunk_cnt = 0;

  for (; i < num_of_chunks; i++, chunk_ptr += SHA1_HASH_SIZE) {
    char *chunk_hash = (char*)malloc(SHA1_HASH_SIZE*2+1);
    hex2ascii(chunk_ptr, SHA1_HASH_SIZE, chunk_hash);
    chunk_hash[SHA1_HASH_SIZE*2] = '\0';

    any_t dummy;
    if (hashmap_get(g->g_config->chunks->has_chunk_map, chunk_hash, &dummy)
            == MAP_OK) {
      ihave_chunk_cnt++;
      console_log("Peer %d: I have chunk %s in local for peer %d",
                  g->g_config->identity, chunk_hash, id);
      memcpy(ihave_payload_ptr, chunk_ptr, SHA1_HASH_SIZE);
      ihave_payload_ptr += SHA1_HASH_SIZE;
    }
  }

  if (ihave_chunk_cnt == 0) {
    return ;
  }

  memcpy(ihave_packet->payload, &ihave_chunk_cnt, sizeof(uint8_t));
  memset(ihave_packet->payload+sizeof(uint8_t), 0, 3*sizeof(uint8_t));
  unsigned int packet_len = HDRSZ + 4*sizeof(uint8_t) + ihave_chunk_cnt*SHA1_HASH_SIZE;
  ihave_packet->hdr->plen = (uint16_t)htons(packet_len);

  /* Send IHAVE packet */
  send_packet(id, ihave_packet, g);
  pkt_free(ihave_packet);
  console_log("Peer %d: Sent IHAVE packet to peer %d", g->g_config->identity, id);
}

void process_ihave_packet(g_state_t *g, packet_t* ih_packet, short id) {
  uint8_t num_of_chunks;
  memcpy(&num_of_chunks, ih_packet->payload, sizeof(uint8_t));

  assert(num_of_chunks > 0);

  uint8_t i = 0;
  uint8_t *chunk_ptr = ih_packet->payload + 4*sizeof(uint8_t);

  for (; i < num_of_chunks; i++, chunk_ptr+=SHA1_HASH_SIZE) {
    char *chunk_hash = (char *) malloc(SHA1_HASH_SIZE * 2 + 1);
    hex2ascii(chunk_ptr, SHA1_HASH_SIZE, chunk_hash);
    chunk_hash[2 * SHA1_HASH_SIZE] = '\0';
    /* Check if there's existing download connection for this chunk.
     * If so, discard this IHAVE packet. */
    any_t dummy;
    if (hashmap_get(g->g_session->nlchunk_located, chunk_hash, &dummy) == MAP_OK) {
      console_log("Peer %d: Discard IHAVE packet from peer %d for chunk %s",
                  g->g_config->identity, id, chunk_hash);
      continue;
    }
    dummy = (any_t) 0;
    hashmap_put(g->g_session->nlchunk_located, chunk_hash, dummy);

    console_log("Peer %d: Peer %d has chunk %s",
                g->g_config->identity, id, chunk_hash);

    /* Send GET packet to corresponding peer */
    packet_t *GET_packet = build_get_packet(chunk_hash);
    try_send_get_packet(id, GET_packet, g);
  }
}
