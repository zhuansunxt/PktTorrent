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
#include <sys/time.h>
#include <unistd.h>

#define DEBUG 1
#define FILE_NAME_LEN 256
#define HASH_STR_LEN 41    /* 2*SHA1_HASH_SIZE+1 */
#define PACKET_LEN 1500
#define PACKET_TYPE_OFFSET 3
#define MAX_PEER_NUM 64     /* TODO: figureout the best config of this */
#define MAX_SEQ_NUM 354     /* CHUNK_SIZE/DATA_PACKET_SIZE + 1 */
#define MAX_DATAPKT_FOR_CHUNK 1024
#define INIT_WINDOW_SIZE 8

#define CHUNK_SIZE (512*1024)
#define DATA_PACKET_SIZE (1500-16)

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

void console_log(const char *fmt, ...);
long get_time_diff(struct timeval *t1, struct timeval *t2);

void try_file(const char *path);

#endif //PACTORRENT_COMMONS_H
