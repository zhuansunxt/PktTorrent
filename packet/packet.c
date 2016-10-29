/**
 * @file packet.c
 * @brief Implementation of packet.h
 * @author Xiaotong Sun <xiaotons@andrew.cmu.edu>
 * @author Longqi Cai   <longqic@andrew.cmu.edu>
 */

#include "packet.h"

packet_t* pkt_new() {
  packet_t* pkt = (packet_t*)malloc(sizeof(packet_t));
  pkt->hdr = (phdr_t*) pkt->raw;
  pkt->payload = pkt->raw + HDRSZ;
  bzero(pkt->raw, PKTSZ);
  return pkt;
}

void pkt_free(packet_t* pkt) {
  free(pkt);
}

void print_packet(packet_t *pkt) {
  console_log("Packet Layout");
  console_log("- Header");
  console_log("--- Magic-number: %u", (uint16_t)ntohs(pkt->hdr->magic));
  console_log("--- Version-number: %u", pkt->hdr->version);
  console_log("--- Type: %u", pkt->hdr->type);
  console_log("--- Header-Len: %u", (uint16_t)ntohs(pkt->hdr->hlen));
  console_log("--- Packert-Len: %u", (uint16_t)ntohs(pkt->hdr->plen));
  console_log("--- SEQ-number: %u", (uint32_t)ntohl(pkt->hdr->seqn));
  console_log("--- ACK-number: %u", (uint32_t)ntohl(pkt->hdr->ackn));
  console_log("- Payload");

  uint8_t type = pkt->hdr->type;
  /* TODO: further print the payload of packet for debugging. */
  switch (type) {
    case 0: // WHOHAS.
      break;
    case 1: // IHAVE.
      break;
    case 2: // GET.
      break;
    case 3: // DATA.
      console_log("--- [DATA Payload]");
      break;
    default:
      console_log("[Error] Invalid packet type");
  }
}