/*
 * bt_parse.h
 *
 * Initial Author: Ed Bardsley <ebardsle+441@andrew.cmu.edu>
 * Class: 15-441 (Spring 2005)
 *
 * Skeleton for 15-441 Project 2 command line and config file parsing
 * stubs.
 *
 */

#ifndef _BT_PARSE_H_
#define _BT_PARSE_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../hashmap/hashmap.h"

#define BT_FILENAME_LEN 255
#define BT_MAX_PEERS 1024

/* Linked-list storing peers' info */
typedef struct bt_peer_s {
  short  id;
  struct sockaddr_in addr;
  struct bt_peer_s *next;
} bt_peer_t;

/* In-memory master-chunk-file */
typedef struct bt_chunks_s {
  char master_data_file[BT_FILENAME_LEN];   // master-data-file path.
  map_t has_chunk_map;                      // chunk --> id.
} bt_chunks_t;

struct bt_config_s {
  /* Command-lind arguements */
  int argc;
  char **argv;
  char  peer_list_file[BT_FILENAME_LEN];    // global peer info file path.
  char  chunk_file[BT_FILENAME_LEN];        // master-chunk-file path.
  char  has_chunk_file[BT_FILENAME_LEN];    // peer's has-chunk-file path.
  int   max_conn;                           // maximum connection.
  short identity;

  /* Global peer info */
  bt_peer_t *peers;

  /* Master-chunk-file info */
  bt_chunks_t *chunks;

  /* Peer's own configuration infomations */
  unsigned short myport;

  char  output_file[BT_FILENAME_LEN];
};
typedef struct bt_config_s bt_config_t;


void bt_init(bt_config_t *c, int argc, char **argv);
void bt_parse_command_line(bt_config_t *c);
void bt_parse_peer_list(bt_config_t *c);
void bt_parse_chunks_info(bt_config_t *c);
void bt_dump_config(bt_config_t *c);
void bt_dump_chunkinfo(bt_config_t *c);
bt_peer_t *bt_peer_info(const bt_config_t *c, int peer_id);

#endif /* _BT_PARSE_H_ */
