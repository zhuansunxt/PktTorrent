/**
 * @file test_packet.c
 * @brief Test driver for packet.
 * @author Xiaotong Sun <xiaotons@andrew.cmu.edu>
 * @author Longqi Cai   <longqic@andrew.cmu.edu>
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "packet.h"

int main(int argc, char** argv) {
  packet_t* packet = malloc(sizeof(packet_t));
  char* raw = "\x51\x3c\x01\x00"
              "\x00\x10\x00\x3c"
              "\xff\xff\x00\x00"
              "\x00\x00\xff\xff";
  memcpy(packet->raw, raw, HDRSZ);
  packet->hdr = (phdr_t*) packet->raw;

  assert( sizeof(phdr_t) == 16 );
  assert( packet->hdr->magic == 15441 );
  assert( packet->hdr->version == 1 );
  assert( packet->hdr->type == 0 );
  assert( packet->hdr->hlen == 0x1000 );
  assert( packet->hdr->plen == 0x3c00 );
  assert( packet->hdr->seqn == 0xffff );
  assert( packet->hdr->ackn == 0xffff0000 );

  printf("%s succeeds!\n", argv[0]);
  return 0;
}
