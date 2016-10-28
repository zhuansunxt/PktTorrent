/**
 * @file download_handler.h
 * @brief download component for both sender and receiver.
 * @author Xiaotong Sun <xiaotons@andrew.cmu.edu>
 * @author Longqi Cai   <longqic@andrew.cmu.edu>
 */

#ifndef PACTORRENT_DOWNLOAD_HANDLER_H
#define PACTORRENT_DOWNLOAD_HANDLER_H

#include "../utilities/commons.h"
#include "../packet/packet.h"
#include "global.h"
#include "download_handler.h"
#include "../utilities/spiffy.h"
#include "../utilities/chunk.h"
#include "../utilities/sha.h"

/* Packet builder */
packet_t *build_get_packet(const char *chunk_hash);
packet_t *build_data_packet(unsigned int seq, size_t data_size, char *data);

void send_packet(short id, packet_t *get_packet, g_state_t *g);
void process_get_packet(g_state_t *g, packet_t *get_packet, short from);

#endif //PACTORRENT_DOWNLOAD_HANDLER_H
