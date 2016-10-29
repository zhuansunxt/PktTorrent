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

/**
 * Iterator to tranverse and print map content.
 * Only int is allowed, though typed as any_t.
 */
int chunk_map_iter(const char* key, any_t val, map_t map) {
  console_log(" ---- <%s, %d>", key, (intptr_t) val);
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
  recv_window->window = queue_new();
  recv_window->max_window_size = INIT_WINDOW_SIZE;
  g->download_conn_pool[peer_id] = recv_window;
}

void free_recv_window(g_state_t *g, short peer_id) {
  queue_free(g->download_conn_pool[peer_id]->window);
  free(g->download_conn_pool[peer_id]);
  g->download_conn_pool[peer_id] = NULL;
}

void init_send_window(g_state_t *g, short peer_id) {
  send_window_t *send_window = (send_window_t*)malloc(sizeof(send_window_t));
  send_window->window = queue_new();
  send_window->max_window_size = INIT_WINDOW_SIZE;
  g->upload_conn_pool[peer_id] = send_window;
}

void free_send_window(g_state_t *g, short peer_id) {
  queue_free(g->upload_conn_pool[peer_id]->window);
  free(g->upload_conn_pool[peer_id]);
  g->download_conn_pool[peer_id] = NULL;
}
