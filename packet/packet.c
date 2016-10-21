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
  return pkt;
}

void pkt_free(packet_t* pkt) {
  free(pkt);
}
