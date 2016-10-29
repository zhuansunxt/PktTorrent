/**
 * @file session.c
 * @brief implementation for user session.
 * @author Xiaotong Sun <xiaotons@andrew.cmu.edu>
 * @author Longqi Cai   <longqic@andrew.cmu.edu>
 */

#include "global.h"

void g_state_init(g_state_t *g) {
  g->peer_socket = -1;
  g->g_config = NULL;
  g->g_session = NULL;

  int i;
  for (i = 0; i < MAX_PEER_NUM; i++)
    g->upload_conn_pool[i] = NULL;
  for (i = 0; i < MAX_PEER_NUM; i++)
    g->download_conn_pool[i] = NULL;
}

void session_init(session_t *s) {
  s->state = NONE;
  s->chunk_map = hashmap_new();
  s->nlchunk_map = hashmap_new();
  bzero(s->output_file, FILE_NAME_LEN);
  s->non_local_chunks = NULL;
}

/* Iterator to tranverse and print lib content */
int chunk_map_iter(const char* key, char* val, map_t map) {
  console_log(" ---- <%s, %s>", key, val);
  return MAP_OK;
}

void dump_session(session_t *s) {
  console_log("[Peer's Current Session Info]");
  console_log(" -- output-file-name: %s", s->output_file);
  console_log(" -- chunk-requested:");
  hashmap_iterate(s->chunk_map, chunk_map_iter, NULL);
}

/* ---------------------- Window related helpers ----------------------*/
void init_recv_window(g_state_t *g, short peer_id) {
  recv_window_t *recv_window = (recv_window_t*)malloc(sizeof(recv_window_t));

  recv_window->max_window_size = INIT_WINDOW_SIZE;
  recv_window->next_packet_expected = 1;
  int i = 0;
  for (; i <= MAX_PEER_NUM; i++)
    recv_window->buffer[i] = NULL;

  g->download_conn_pool[peer_id] = recv_window;
}

void free_recv_window(g_state_t *g, short peer_id) {
  free(g->download_conn_pool[peer_id]);
  g->download_conn_pool[peer_id] = NULL;
}

void init_send_window(g_state_t *g, short peer_id) {
  send_window_t *send_window = (send_window_t*)malloc(sizeof(send_window_t));

  send_window->max_window_size = INIT_WINDOW_SIZE;
  send_window->last_packet_acked = 0;
  send_window->last_packet_sent = 0;
  send_window->last_packet_available = INIT_WINDOW_SIZE;
  int i = 0;
  for (; i <= MAX_PEER_NUM; i++)
    send_window->buffer[i] = NULL;
  send_window->dup_ack = hashmap_new();

  g->upload_conn_pool[peer_id] = send_window;
}

void free_send_window(g_state_t *g, short peer_id) {
  hashmap_free(g->upload_conn_pool[peer_id]->dup_ack);
  free(g->upload_conn_pool[peer_id]);
  g->download_conn_pool[peer_id] = NULL;
}
