//
// Created by XiaotongSun on 16/10/23.
//

#include "handler.h"
#include "location_handler.h"
#include "../utilities/commons.h"
#include "../packet/packet.h"

void process_packet(g_state_t *g, char *buf, short id) {

  packet_t *packet = pkt_new();
  memcpy(packet->raw, buf, HDRSZ);
  uint8_t type = packet->hdr->type;

#ifdef DEBUG
  console_log("*******Receiving Packet*********");
  console_log("packet magic number: %u", (uint16_t)ntohs(packet->hdr->magic));
  console_log("packet version number: %u", packet->hdr->version);
  console_log("packet type : %u", type);
  console_log("packet header length: %u", (uint16_t)ntohs(packet->hdr->hlen));
  console_log("packet packet length: %u", (uint16_t)ntohs(packet->hdr->plen));
  console_log("packet seq number: %u", (uint32_t)ntohl(packet->hdr->seqn));
  console_log("packet ack number: %u", (uint32_t)ntohl(packet->hdr->ackn));
#endif

  memcpy(packet->payload, buf+HDRSZ, ntohs(packet->hdr->plen)-HDRSZ);

  switch (type) {
    case 0:
      console_log("Peer %d: Receiving WHOHAS packet", g->g_config->identity);
      process_who_has_packet(g, packet, id);
      break;
    case 1:
      console_log("Peer %d: Receiving IHAVE packet", g->g_config->identity);
      process_ihave_packet(g, packet, id);
      break;
    case 2:
      console_log("Peer %d: Receiving  GET packet", g->g_config->identity);
      break;
    case 3:
      console_log("Peer %d: Receiving DATA packet", g->g_config->identity);
      break;
    case 4:
      console_log("Peer %d: Receiving ACK packet", g->g_config->identity);
      break;
    case 5:
      console_log("Peer %d: Receiving DENIED packet", g->g_config->identity);
      break;
    default:
      fprintf(stderr, "Peer %d: Unknown type of packet. Drop it", g->g_config->identity);
      return;
  }
}