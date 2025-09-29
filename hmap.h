
#ifndef HASH_MAP_H_
#define HASH_MAP_H_

#include <stddef.h> // size_t


struct hash_map {
  size_t cap, nm_ent;
  struct map_ent **list; /* buckets */
};

extern int hash_map_alloc(
  struct hash_map *,
  size_t
);
extern void hash_map_free(struct hash_map *);
extern int hash_map_insert_kv(
  struct hash_map *,
  const char *,
  int
);
extern int *hash_map_fetch_node(
  const struct hash_map *,
  const char *
);

#endif

