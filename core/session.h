/**
 * @file session.h
 * @brief user_session as global states.
 * @author Xiaotong Sun <xiaotons@andrew.cmu.edu>
 * @author Longqi Cai   <longqic@andrew.cmu.edu>
 */

#ifndef PACTORRENT_SESSION_H
#define PACTORRENT_SESSION_H

#include "../hashmap/hashmap.h"
#include "../utilities/bt_parse.h"
#include "../utilities/commons.h"

typedef enum session_state_enum{
  NONE,
  AWAITING_WHOHAS,
  AWAITING_IHAVE,
  AWAITING_GET,
  AWAITING_DATA,
  AWAITING_ACK
} session_state_t;

typedef struct session_nlchunks_s {
  char chunk_hash[HASH_STR_LEN];
  struct session_nlchunks_s *next;
} session_nlchunk_t;

typedef struct session_s {
  session_state_t state;
  map_t chunk_map;    /* user_requeste_chunk -> chunk_id */
  map_t nlchunk_map;  /* non_local_chunk -> peer_id */
  char output_file[FILE_NAME_LEN];
  session_nlchunk_t *non_local_chunks;  /* list of non_local_chunks */
} session_t;

typedef struct g_state_s {
  int peer_socket;
  bt_config_t *g_config;
  session_t *g_session;
} g_state_t;

void session_init(session_t *s);
void dump_session(session_t *s);

#endif //PACTORRENT_SESSION_H
