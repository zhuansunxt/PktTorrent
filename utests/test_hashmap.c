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

int copy_map(const char* key, int* val, map_t map2) {
  hashmap_put(map2, key, val);
  return MAP_OK;
}

int main(int argc, char** argv) {
  int i;
  map_t map = hashmap_new();
  char* k1 = "abc";
  any_t v1 = 0;
  char* k2 = "bcd";
  any_t v2 = 1;
  char* k3 = "cde";
  any_t v3 = 2;

  /* test put */
  hashmap_put(map, k1, v1);
  hashmap_put(map, k2, v2);
  hashmap_put(map, k3, v3);

  /* test get */
  any_t vv1 = -1, vv2 = -1, vv3 = -1;
  assert( hashmap_get(map, k1, &vv1) == MAP_OK && vv1 == v1 );
  assert( hashmap_get(map, k2, &vv2) == MAP_OK && vv2 == v2 );
  assert( hashmap_get(map, k3, &vv3) == MAP_OK && vv3 == v3 );

  /* test remove */
  vv1 = -1; vv2 = -1; vv3 = -1;
  hashmap_remove(map, k1);
  assert( hashmap_get(map, k1, &vv1) == MAP_MISSING );
  assert( hashmap_get(map, k2, &vv2) == MAP_OK && vv2 == v2 );
  assert( hashmap_get(map, k3, &vv3) == MAP_OK && vv3 == v3 );

  /* test iterate */
  map_t map2 = hashmap_new();
  vv1 = -1; vv2 = -1; vv3 = -1;
  hashmap_iterate(map, copy_map, map2);
  assert( hashmap_get(map2, k1, &vv1) == MAP_MISSING );
  assert( hashmap_get(map2, k2, &vv2) == MAP_OK && vv2 == v2 );
  assert( hashmap_get(map2, k3, &vv3) == MAP_OK && vv3 == v3 );

  printf("%s succeeds!\n", argv[0]);

  hashmap_free(map);
  return 0;
}
