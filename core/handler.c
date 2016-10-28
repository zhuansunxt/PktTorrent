//
// Created by XiaotongSun on 16/10/23.
//

#include "handler.h"
#include "location_handler.h"
#include "../utilities/commons.h"
#include "../packet/packet.h"

/**
 * Process incoming packet from peers.
 * @param g current global state.
 * @param buf incoming data buffer.
 * @param id incoming peer id.
 */
void process_packet(g_state_t *g, char *buf, short id) {

  packet_t *packet = pkt_new();
  memcpy(packet->raw, buf, HDRSZ);
  uint8_t type = packet->hdr->type;
  
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
      process_get_packet(g, packet, id);
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

  pkt_free(packet);
}