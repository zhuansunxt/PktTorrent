/**
 * @file user_handler.c
 * @brief implementation for user input handler.
 * @author Xiaotong Sun <xiaotons@andrew.cmu.edu>
 * @author Longqi Cai   <longqic@andrew.cmu.edu>
 */

#include <stdio.h>
#include <assert.h>
#include "../utilities/commons.h"
#include "user_handler.h"
#include "../utilities/commons.h"

void process_get(char *chunkfile, char *outputfile, g_state_t *g){
  FILE *get_chunk_f = NULL;
  char *line = NULL;
  size_t len;

  strcpy(g->g_session->output_file, outputfile);
  char *temp_output_name = "tmp.output";
  strcpy(g->g_session->temp_output_file, temp_output_name);

  get_chunk_f = fopen(chunkfile, "r");
  if (get_chunk_f == NULL) {
    fprintf(stderr, "Error when accessing user's chunk file\n");
    return ;
  }

  while(getline(&line, &len, get_chunk_f) != -1) {
    if (line[0] == '#') continue;
    // fmt: id hash
    // id is value; hash is key
    char* key = (char*) malloc(HASHSTR_SZ+1);
    any_t value;
    assert(sscanf(line, "%ld %s", (intptr_t*) &value, key) != 0);
    hashmap_put(g->g_session->chunk_map, key, value);
    if (hashmap_get(g->g_config->chunks->has_chunk_map,
                    key, &value) == MAP_MISSING) {
      g->g_session->state = AWAITING_WHOHAS;
      session_nlchunk_t *nlchunk = (session_nlchunk_t*)malloc(sizeof(session_nlchunk_t));
      assert(nlchunk != NULL);
      strcpy(nlchunk->chunk_hash, key);
      nlchunk->next = g->g_session->non_local_chunks;
      g->g_session->non_local_chunks = nlchunk;

      // Keep record of non-local chunks in a map as well.
      // For further checking whether all non-local chunks are downloaded in the future.
      any_t dummy = (any_t) 0;
      hashmap_put(g->g_session->nlchunk_map, key, dummy);
    }
  }
  free(line);
  fclose(get_chunk_f);

#ifdef DEBUG_
  bt_dump_config(g->g_config);
  bt_dump_chunkinfo(g->g_config);
  dump_session(g->g_session);
  session_nlchunk_t *p;
  for (p = g->g_session->non_local_chunks; p; p = p->next)
    console_log("Non-local-chunk: %s", p->chunk_hash);
#endif
}
