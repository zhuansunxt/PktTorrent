//
// Created by XiaotongSun on 16/10/22.
//

#include <stdio.h>
#include <assert.h>
#include "../utilities/commons.h"
#include "user_handler.h"
#include "../utilities/commons.h"

void process_get(char *chunkfile, char *outputfile, g_state_t *g){
  FILE *get_chunk_f = NULL;
  char *line = NULL;
  int len;

  strcpy(g->g_session->output_file, outputfile);

  get_chunk_f = fopen(chunkfile, "r");
  if (get_chunk_f == NULL) {
    fprintf(stderr, "Error when accessing user's chunk file\n");
    return ;
  }

  while(getline(&line, &len, get_chunk_f) != -1) {
    if (line[0] == '#') continue;
    char *key = (char*)malloc(8);
    char *value = (char*)malloc(64);
    assert(sscanf(line, "%s %s", key, value) != 0);
    hashmap_put(g->g_session->chunk_map, key, value);
    char *chunk_hash;
    if (hashmap_get(g->g_config->chunks->has_chunk_map,
                    value, (any_t*)chunk_hash) == MAP_MISSING) {
      g->g_session->state = AWAITING_WHOHAS;
      session_nlchunk_t *nlchunk = (session_nlchunk_t*)malloc(sizeof(session_nlchunk_t));
      assert(nlchunk != NULL);
      strcpy(nlchunk->chunk_hash, value);
      nlchunk->next = g->g_session->non_local_chunks;
      g->g_session->non_local_chunks = nlchunk;
    }
  }
  free(line);
  fclose(get_chunk_f);
  console_log("2");

#ifdef DEBUG
  bt_dump_config(g->g_config);
  bt_dump_chunkinfo(g->g_config);
  dump_session(g->g_session);
  session_nlchunk_t *p;
  for (p = g->g_session->non_local_chunks; p; p = p->next)
    console_log("Non-local-chunk: %s", p->chunk_hash);
#endif
}
