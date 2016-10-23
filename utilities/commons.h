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

#define DEBUG 1
#define FILE_NAME_LEN 256
#define HASH_STR_LEN 64

void console_log(const char *fmt, ...);

#endif //PACTORRENT_COMMONS_H
