/**
 * @file commons.h
 * @brief Global common helpers.
 * @author Xiaotong Sun <xiaotons@andrew.cmu.edu>
 * @author Longqi Cai   <longqic@andrew.cmu.edu>
 */

#ifndef PACTORRENT_COMMONS_H
#define PACTORRENT_COMMONS_H

#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define DEBUG 1
#define FILE_NAME_LEN 256
#define HASH_STR_LEN 64
#define PACKET_LEN 1500
#define PACKET_TYPE_OFFSET 3
#define MAX_PEER_NUM 64     /* TODO: figureout the best config of this */
#define INIT_WINDOW_SIZE 8;

#define CHUNK_SIZE (512*1024)
#define DATA_PACKET_SIZE (1500-16)

void console_log(const char *fmt, ...);

#endif //PACTORRENT_COMMONS_H
