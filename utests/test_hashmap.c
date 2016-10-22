/**
 * @file test_hashmap.c
 * @brief Test driver for hashmap.[h|c]
 * @author Xiaotong Sun <xiaotons@andrew.cmu.edu>
 * @author Longqi Cai   <longqic@andrew.cmu.edu>
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "hashmap.h"

int main(int argc, char** argv) {
  map_t map = hashmap_new();
  char* k1 = "abc";
  int*  v1 = malloc(sizeof(int)); *v1 = 1;
  char* k2 = "bcde";
  int*  v2 = malloc(sizeof(int)); *v2 = 2;

  hashmap_put(map, k1, v1);
  hashmap_put(map, k2, v2);

  int* vv1;
  int* vv2;
  assert( hashmap_get(map, k1, (any_t*) &vv1) == MAP_OK && vv1 == v1 );
  assert( hashmap_get(map, k2, (any_t*) &vv2) == MAP_OK && vv2 == v2 );

  vv1 = NULL; vv2 = NULL;
  hashmap_remove(map, k1);
  assert( hashmap_get(map, k1, (any_t*) &vv1) == MAP_MISSING );
  assert( hashmap_get(map, k2, (any_t*) &vv2) == MAP_OK && vv2 == v2 );

  printf("%s succeeds!\n", argv[0]);

  hashmap_free(map);
  return 0;
}
