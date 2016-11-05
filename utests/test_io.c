/**
 * @file test_io.c
 * @brief Test driver for io functions.
 * @author XiaotongSun <xiaotons@andrew.cmu.edu>
 * @author LongqiCai <longqic@andrew.cmu.edu>
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commons.h"
#include "hashmap.h"
#include "io.h"

int main(int argc, char** argv){
  size_t i, j;
  map_t master_chunk_map = hashmap_new();
  map_t user_chunk_map = hashmap_new();

  hashmap_put(master_chunk_map, "00", (any_t) 0);
  hashmap_put(master_chunk_map, "01", (any_t) 1);
  hashmap_put(master_chunk_map, "02", (any_t) 2);
  hashmap_put(master_chunk_map, "03", (any_t) 3);
  hashmap_put(master_chunk_map, "04", (any_t) 4);
  hashmap_put(master_chunk_map, "05", (any_t) 5);
  hashmap_put(master_chunk_map, "06", (any_t) 6);
  hashmap_put(master_chunk_map, "07", (any_t) 7);

  hashmap_put(user_chunk_map, "00", (any_t) 0);
  hashmap_put(user_chunk_map, "02", (any_t) 1);
  hashmap_put(user_chunk_map, "04", (any_t) 2);
  hashmap_put(user_chunk_map, "06", (any_t) 3);

  static char buf_master[CHUNK_SIZE+1];
  const char* path_master = "/tmp/test_io.master";
  FILE* fp_master = fopen(path_master, "w");
  for (i = 0; i < 8; i++) {
    memset(buf_master, i, CHUNK_SIZE);
    fwrite(buf_master, CHUNK_SIZE, 1, fp_master);
  }
  fclose(fp_master);

  const char* path_user = "/tmp/test_io.user";
  assemble_chunks(path_master, master_chunk_map,
                  path_user, user_chunk_map);

  static char buf_user[CHUNK_SIZE+1];
  FILE* fp_user = fopen(path_user, "r");
  for (i = 0; i < 4; i++) {
    fread(buf_user, CHUNK_SIZE, 1, fp_user);
    for (j = 0; j < CHUNK_SIZE; j++)
      assert( buf_user[j] == i*2 );
  }
  fclose(fp_user);

  hashmap_free(master_chunk_map);
  hashmap_free(user_chunk_map);
  printf("%s succeeds!\n", argv[0]);
  return 0;
}
