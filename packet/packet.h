/**
 * @file packet.h
 * @brief Packet helper.
 * @author Xiaotong Sun <xiaotons@andrew.cmu.edu>
 * @author Longqi Cai   <longqic@andrew.cmu.edu>
 */

#ifndef PACKET_H
#define PACKET_H

#include <stdlib.h>
#include <stdint.h>

#define PKTSZ 1500

typedef struct phdr_s {
  uint16_t magic; uint8_t version, type;
  uint16_t hlen; uint16_t plen;
  uint32_t seqn;
  uint32_t ackn;
} __attribute__((packed)) phdr_t;

#define HDRSZ sizeof(phdr_t)

typedef struct packet_s {
  char    raw[PKTSZ];
  phdr_t* hdr;
  void*   payload;
} packet_t;

packet_t* pkt_new();
void pkt_free(packet_t* pkt);
void print_packet(packet_t* pkt);

#endif // PACKET_H
