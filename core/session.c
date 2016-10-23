//
// Created by XiaotongSun on 16/10/22.
//

#include "session.h"

void session_init(session_t *s) {
  s->state = NONE;
  s->chunk_map = hashmap_new();
  bzero(s->output_file, FILE_NAME_LEN);
  s->non_local_chunks = NULL;
}

int chunk_map_iter(const char* key, char* val, map_t map) {
  console_log("--- <%s, %s>", key, val);
  return MAP_OK;
}

void dump_session(session_t *s) {
  console_log("**********PacTorrent Current Session Info***********");
  console_log("output-file-name: %s", s->output_file);
  console_log("chunk-requested:");
  hashmap_iterate(s->chunk_map, chunk_map_iter, NULL);
}
