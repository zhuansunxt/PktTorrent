/**
 * @file io.c
 * @brief Implementation of io.h
 * @author Xiaotong Sun <xiaotons@andrew.cmu.edu>
 * @author Longqi Cai   <longqic@andrew.cmu.edu>
 */

#include <stdio.h>
#include "utilities/io.h"
#include "utilities/commons.h"

typedef struct _assemble_s {
  FILE* fp_master;
  FILE* fp_user;
  map_t master_chunk_map;
} _assemble_t;

// not thread safe
int _iter_assemble(const char* key, any_t user_id, any_t data) {
  static char buffer[CHUNK_SIZE+1];
  _assemble_t* assemble = (_assemble_t*) data;
  any_t master_id;
  hashmap_get(assemble->master_chunk_map, key, &master_id);

  size_t offset_master = (intptr_t) master_id * CHUNK_SIZE;
  size_t offset_user = (intptr_t) user_id * CHUNK_SIZE;
  fseek(assemble->fp_master, offset_master, SEEK_SET);
  fseek(assemble->fp_user, offset_user, SEEK_SET);
  fread(buffer, CHUNK_SIZE, 1, assemble->fp_master);
  fwrite(buffer, CHUNK_SIZE, 1, assemble->fp_user);

  return MAP_OK;
}

void assemble_chunks(const char* master_data_file, const map_t master_chunk_map,
                     const char* output_file, const map_t chunk_map) {
  _assemble_t assemble;
  assemble.fp_master = fopen(master_data_file, "r");
  assemble.fp_user = fopen(output_file, "w+");
  assemble.master_chunk_map = master_chunk_map;
  hashmap_iterate(chunk_map, _iter_assemble, &assemble);
  fclose(assemble.fp_master);
  fclose(assemble.fp_user);
}
