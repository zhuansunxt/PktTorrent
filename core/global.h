/**
 * @file session.h
 * @brief user_session as global states.
 * @author Xiaotong Sun <xiaotons@andrew.cmu.edu>
 * @author Longqi Cai   <longqic@andrew.cmu.edu>
 */

#ifndef PACTORRENT_SESSION_H
#define PACTORRENT_SESSION_H

#include "../lib/hashmap.h"
#include "bt_parse.h"
#include "../utilities/commons.h"
#include "../lib/queue.h"
#include "../packet/packet.h"

/**
 * State definition for session.
 */
typedef enum session_state_enum{
  NONE,
  AWAITING_WHOHAS,
  AWAITING_IHAVE,
  AWAITING_GET,
  AWAITING_DATA,
  AWAITING_ACK,
  DONE,
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
 * Window type used by receiver in reliable network transfer.
 */
typedef struct recv_window_s {
  packet_t* buffer[MAX_SEQ_NUM+1];  // buffer already received DATA packet.
  size_t max_window_size;           // upper bound on current window size.
  uint32_t next_packet_expected;    // next expected packet's sequence number.
} recv_window_t;

/**
 * Window type used by sender in reliable network transfer.
 * The difference agains recv_window_t is it contains congestion
 * control mechanism.
 */
typedef struct send_window_s {
  packet_t *buffer[MAX_SEQ_NUM+1];  // buffer all DATA packet.
  size_t max_window_size;           // upper bound on current window size.
  uint32_t last_packet_acked;       // last packet that get ACKed.
  uint32_t last_packet_sent;        // last packet that is sent out.
  uint32_t last_packet_available;   // serves as window boundary.
  map_t dup_ack;                    // keep track of duplicate ack.
} send_window_t;

/**
 * Global states shared by all components.
 */
typedef struct g_state_s {
  int peer_socket;          // socket listener for peers.
  bt_config_t *g_config;    // configurations.
  session_t *g_session;     // session state.
  send_window_t * upload_conn_pool[MAX_PEER_NUM];     // pool for downloading connections.
  recv_window_t * download_conn_pool[MAX_PEER_NUM];   // pool for uploading connections.
} g_state_t;

void g_state_init(g_state_t *g);
void session_init(session_t *s);
void dump_session(session_t *s);

/* Window helper */
void init_recv_window(g_state_t *g, short peer_id);
void free_recv_window(g_state_t *g, short peer_id);
void init_send_window(g_state_t *g, short peer_id);
void free_send_window(g_state_t *g, short peer_id);

#endif //PACTORRENT_SESSION_H
