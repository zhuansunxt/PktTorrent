/**
 * @file session.h
 * @brief user_session as global states.
 * @author Xiaotong Sun <xiaotons@andrew.cmu.edu>
 * @author Longqi Cai   <longqic@andrew.cmu.edu>
 */

#ifndef PACTORRENT_SESSION_H
#define PACTORRENT_SESSION_H

#include "hashmap.h"
#include "bt_parse.h"
#include "commons.h"

/**
 * State definition for session.
 */
typedef enum session_state_enum{
  NONE,
  AWAITING_WHOHAS,
  AWAITING_IHAVE,
  AWAITING_GET,
  AWAITING_DATA,
  AWAITING_ACK
} session_state_t;

/**
 * Linked list storing non-local chunks for this peer.
 */
typedef struct session_nlchunks_s {
  char chunk_hash[HASH_STR_LEN];
  struct session_nlchunks_s *next;
} session_nlchunk_t;

/**
 * Session state.
 * A session contains data structures and states for
 * the whole peer interaction process for one user request.
 */
typedef struct session_s {
  session_state_t state;                // current state of the session.
  map_t chunk_map;                      // user_requested_chunk -> chunk_id.
  map_t nlchunk_map;                    // non_local_chunk -> peer owner.
  char output_file[FILE_NAME_LEN];      // user designated output file path.
  session_nlchunk_t *non_local_chunks;  // list of non-local chunks.
} session_t;

/**
 * Global states shared by all components.
 */
typedef struct g_state_s {
  int peer_socket;          // socket listener for peers.
  bt_config_t *g_config;    // configurations.
  session_t *g_session;     // session state.
} g_state_t;

void session_init(session_t *s);
void dump_session(session_t *s);

#endif //PACTORRENT_SESSION_H
