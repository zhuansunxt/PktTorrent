/**
 * @file user_handler.h
 * @brief helper functions for user input handling
 * @author Xiaotong Sun <xiaotons@andrew.cmu.edu>
 * @author Longqi Cai   <longqic@andrew.cmu.edu>
 */

#ifndef PACTORRENT_USER_HANDLER_H
#define PACTORRENT_USER_HANDLER_H

#include "session.h"

void process_get(char *chunkfile, char *outputfile, g_state_t *g);

#endif //PACTORRENT_USER_HANDLER_H
