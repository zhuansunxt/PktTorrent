/**
 * @file download_handler.c
 * @brief Implementation for download component for the system.
 * @author Xiaotong Sun <xiaotons@andrew.cmu.edu>
 * @author Longqi Cai   <longqic@andrew.cmu.edu>
 */

#include "download_handler.h"

/**
 * GET packet contains only the chunk hash for the chunk
 * the client wants to fetch.
 * @param chunk_hash  string of hash_value of chunk.
 * @return built GET packet.
 */
packet_t *build_get_packet(const char *chunk_hash) {
  packet_t *get_packet = pkt_new();
  get_packet->hdr->magic = htons(15441);
  get_packet->hdr->version = 1;
  get_packet->hdr->type = 2;
  get_packet->hdr->hlen = htons(HDRSZ);
  get_packet->hdr->plen = htons(HDRSZ+SHA1_HASH_SIZE);
  get_packet->hdr->seqn = htonl(0);
  get_packet->hdr->ackn = htonl(0);

  /* Copy chunk hash to packet. */
  char hash_hex[SHA1_HASH_SIZE];
  ascii2hex(chunk_hash, SHA1_HASH_SIZE*2, (uint8_t*)hash_hex);
  memcpy(get_packet->payload, hash_hex, SHA1_HASH_SIZE);
  return get_packet;
}

/**
 * Construct DATA packet.
 * @param seq seqeunce number.
 * @param data_size size of attached data.
 * @param data pointer to attached data.
 * @return built DATA packet.
 */
packet_t *build_data_packet(unsigned int seq, size_t data_size, char *data) {
  packet_t *data_packet;
  uint16_t packet_len = HDRSZ + (uint16_t)data_size;

  data_packet->hdr->magic = htons(15441);
  data_packet->hdr->version = 1;
  data_packet->hdr->type = 3;                       // packet type: DATA
  data_packet->hdr->hlen = htons(HDRSZ);
  data_packet->hdr->plen = htons(packet_len);
  data_packet->hdr->seqn = htonl((uint32_t)seq);
  data_packet->hdr->ackn = htonl(0);

  memcpy(data_packet->payload, data, data_size);
  return data_packet;
}

/**
 * Send GET packet to designated peer.
 * The invoker of this function is the receiver in downloading process.
 * @param id designated peer's id.
 * @param get_packet GET packet
 * @param g global state to retrieve socket and peers info
 */
//TODO: set timeout for GET packet.
void send_packet(short id, packet_t *get_packet, g_state_t *g){
  bt_peer_t *peer = bt_peer_info(g->g_config, id);
  spiffy_sendto(g->peer_socket, get_packet->raw, ntohs(get_packet->hdr->plen),
                0, (struct sockaddr *)&(peer->addr), sizeof(peer->addr));
}

/**
 * Process incoming GET packet from peer.
 * The invoker of this function is the sender in downloading process.
 * @param g current global state.
 * @param get_packet GET packet.
 * @param from peer id.
 */
void process_get_packet(g_state_t *g, packet_t *get_packet, short from) {
  /* Retrieve hashed chunk */
  char *chunk_ptr = (char*)get_packet->payload;
  char chunk_hash[2*SHA1_HASH_SIZE+1];
  hex2ascii(chunk_ptr, SHA1_HASH_SIZE, chunk_hash);
  chunk_hash[2*SHA1_HASH_SIZE] = '\0';

  char *dummy;
  if (hashmap_get(g->g_config->chunks->has_chunk_map, chunk_hash, (any_t*)&dummy)
          == MAP_MISSING) {
    console_log("Peer %d: received GET packet for nl-chunk %s, drop it!",
                g->g_config->identity, chunk_hash);
    return;
  }

  console_log("Peer %d: received GET packet for chunk %s", g->g_config->identity, chunk_hash);
}
