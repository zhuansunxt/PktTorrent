/**
 * @file location_handler.h
 * @brief helper functions for locating locally missing files.
 * @author Xiaotong Sun <xiaotons@andrew.cmu.edu>
 * @author Longqi Cai   <longqic@andrew.cmu.edu>
 */

#ifndef PACTORRENT_LOCATION_HANDLER_H
#define PACTORRENT_LOCATION_HANDLER_H

#include "../utilities/commons.h"
#include "../packet/packet.h"
#include "session.h"

void ask_peers_who_has(g_state_t *g);
void broadcast_who_has_packets(g_state_t *g, packet_t* wh_packet);
packet_t* build_who_has_packet(g_state_t *g);
void process_who_has_packet(g_state_t *g, packet_t* wh_packet, short id);
void process_ihave_packet(g_state_t *g, packet_t* ih_packet, short id);


#endif //PACTORRENT_LOCATION_HANDLER_H
