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

void console_log(const char *fmt, ...);

#endif //PACTORRENT_COMMONS_H
