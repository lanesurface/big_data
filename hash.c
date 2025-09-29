
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> // -lm

#define F_BUFF_SZ 256  /* size of read buffer */
#define INIT_CAP  10   /* number of initial bucket entries */
#define LOAD_FTR  3    /* bounds the min number of buckets */
#define BRK_CHR   " ,;.?!\"()[]\r\n"

#ifdef __GNUC__
#define WEAK_DEF __attribute__((weak))
#else 
#define WEAK_DEF
#endif

extern WEAK_DEF uint32_t hash(
  const char *, 
  size_t, 
  size_t *
);

struct map_ent {
  char *key;
  int value;
  /* TODO: store locality info */
  struct map_ent *next;
};

struct hash_map {
  size_t cap, nm_ent;
  struct map_ent **list; /* buckets */
};

static struct map_ent *hash_map_create_ent(
  const char *,
  int
);

static void hash_map_free_ent(struct map_ent *);

static int 
hash_map_alloc(struct hash_map *h_map, size_t sz)
{
  struct map_ent **m_bk;

  if (h_map) {
    m_bk=calloc(sz,sizeof(*m_bk));
    if (m_bk) {
      /* initially, all buckets allocated but NULL */
      h_map->list=m_bk;
      h_map->cap=sz;
      h_map->nm_ent=0;
      return 0;
    }
  }
  return -1;
}

static void
hash_map_free(struct hash_map *h_map)
{
  struct map_ent *m_ent,*m_next_ent;

  if (h_map) {
    for (size_t i=0; i<h_map->cap; i++) {
      m_ent=h_map->list[i];
      while (m_ent) {
        m_next_ent=m_ent->next;
        hash_map_free_ent(m_ent);
        m_ent=m_next_ent;
      }
    }
    h_map->nm_ent=0, h_map->cap=0;
    free(h_map->list);
    /* avoid dangling reference */
    h_map->list=NULL;
  }
}

static int hash_map_insert_node(
  struct hash_map *,
  struct map_ent *
);

static int
hash_map_rehash(struct hash_map *h_map)
{
  static struct hash_map new_map;
  struct map_ent *m_ent,*m_next_ent;
  size_t new_cap;
  int r_val;

  if (h_map) {
    if (h_map->nm_ent>=LOAD_FTR*h_map->cap) { /* unlikely */
      /* build new hash_map, then swap */
      new_cap=2*h_map->cap;
      r_val=hash_map_alloc(&new_map,new_cap);
      if (r_val) return r_val;
      for (size_t i=0; i<h_map->cap; i++) {
        m_ent=h_map->list[i];
        while (m_ent) {
          m_next_ent=m_ent->next;
          m_ent->next=NULL;
          r_val=hash_map_insert_node(&new_map,m_ent);
          if (r_val) return r_val;
          m_ent=m_next_ent;
        }
      }
      /* don't call hash_map_free here since we still need the nodes */
      free(h_map->list);
      *h_map=new_map;
    }
    return 0;
  }
  return -1; /* HM_ERR_INV_ARG */
}

static int 
hash_map_insert_node(struct hash_map *h_map, struct map_ent *m_ent)
{
  uint32_t h;
  size_t n;

  if (h_map && m_ent) {
    h=hash(m_ent->key,h_map->cap,&n);
    m_ent->next=h_map->list[h];
    h_map->list[h]=m_ent;
    h_map->nm_ent++;
    return hash_map_rehash(h_map);
  }
  return -1;
}

static int
hash_map_insert_kv(struct hash_map *h_map, const char *k, int v) 
{
  struct map_ent *m_ent;

  if (h_map && k) {
    m_ent=hash_map_create_ent(k,v);
    if (m_ent) return hash_map_insert_node(
      h_map,
      m_ent
      );
    return -1; /* HM_ERR_NO_MEM */
  }
  return -1; /* HM_ERR_INV_ARG */
}

static int *
hash_map_fetch_node(const struct hash_map *h_map, const char *k)
{
  struct map_ent *m_ent;
  uint32_t h;
  size_t n;
  int k_cmp;

  if (h_map) {
    h=hash(k,h_map->cap,&n);
    m_ent=h_map->list[h];
    while (m_ent) {
      k_cmp=strcmp(m_ent->key,k);
      if (k_cmp==0) return &(m_ent->value);
      /* ^..!! leaking reference, ptr invalid after map free !! */
      m_ent=m_ent->next;
    }
  }
  return NULL;
}

/* generate list containing number of collisions */
static int 
hash_map_cnt_bk(const struct hash_map *h_map, int bk_n[])
{
  struct map_ent *m_ent;
  int n;
  
  if (h_map && bk_n) {
    for (size_t i=0; i<h_map->cap; i++) {
      n=0, m_ent=h_map->list[i];
      while (m_ent) {
        n++;
        m_ent=m_ent->next;
      }
      bk_n[i]=n;
    }
    return 0;
  }
  return -1;
}

static int 
hash_map_print(const struct hash_map *h_map)
{
  struct map_ent *m_ent;

  if (h_map) {
    for (size_t i=0; i<h_map->cap; i++) {
      printf("-- bucket %zu --\n",i);
      m_ent=h_map->list[i];
      while (m_ent) {
        printf(
          "k=<%s>,cnt=%d\n",
          m_ent->key,
          m_ent->value
          );
        m_ent=m_ent->next;
      }
    }
  }
  return -1;
}

static struct map_ent *
hash_map_create_ent(const char *k, int v)
{
  struct map_ent *m_ent;
  char *new_k;

  if (k) {
    m_ent=calloc(1,sizeof(*m_ent));
    if (m_ent) {
      new_k=strdup(k);
      if (new_k) {
        m_ent->key=new_k;
        m_ent->value=v;
        return m_ent;
      } else {
        free(m_ent);
      }
    }
  }
  return NULL;
}

static void 
hash_map_free_ent(struct map_ent *m_ent) 
{
  if (m_ent) {
    if (m_ent->key) free(m_ent->key);
    free(m_ent);
  }
}

static int 
tok_file(const char *fl_name, struct hash_map *h_map)
{
  FILE *fp;
  char file_buff[F_BUFF_SZ], *sv;
  int r_val, *v_ptr;

  fp=fopen(fl_name,"r");
  if (fp && h_map) {
    sv=fgets(file_buff,F_BUFF_SZ,fp);
    while (sv && !feof(fp)) {
      sv=strtok(sv,BRK_CHR);
      while (sv) {
        v_ptr=hash_map_fetch_node(h_map,sv);
        if (v_ptr) (*v_ptr)++;
        else {
          r_val=hash_map_insert_kv(h_map,sv,1);
          if (r_val) return r_val;
        }
        sv=strtok(NULL,BRK_CHR);
      }
      sv=fgets(file_buff,F_BUFF_SZ,fp);
    }
  }
  return 0;
}

int
main(int argc, char *argv[])
{
  struct hash_map h_map;
  int *bk_n, *v_ptr, r_val;
  char *key;

  if (argc>2) {
    r_val=hash_map_alloc(&h_map,INIT_CAP);
    if (!r_val) {
      key=argv[1];
      for (int i=2; i<argc; i++) {
        r_val=tok_file(argv[i],&h_map);
        if (r_val) {
          fprintf(stderr,"[ERROR]: %d\n",r_val);
          return r_val;
        }
      }
      v_ptr=hash_map_fetch_node(&h_map,key);
      if (v_ptr) printf(
        "%s appears %d times\n",
        key,
        *v_ptr 
        );
      else puts("No matches found");
    }
  } else {
    fprintf(stderr, "USAGE: hash <key> <file_0> ... <file_N>\n");
    exit(-1);
  }
#if 0
  /* export the number of nodes per bucket as a csv */
  bk_n=calloc(sizeof(*bk_n),h_map.cap);
  if (bk_n) {
    hash_map_cnt_bk(&h_map,bk_n);
    for (size_t i=0; i<h_map.cap; i++) printf(
      "%d\n",
      bk_n[i]
      );
  }
#endif
  hash_map_free(&h_map);

  return r_val;
}

uint32_t
hash(const char *key, size_t cap, size_t *n)
{
  uint32_t h=0;

  if (n) {
    *n=0;
    while (key[*n]) {
      h=(h<<*n)^key[*n];
      (*n)++;
    }
  }
  return h%cap;
}

#if 0
uint32_t
hash(const char *key, size_t cap, size_t *n)
{
  uint32_t a=0;

  if (n) {
    *n=0;
    while (key[*n]) {
      a+=key[*n];
      (*n)++;
    }
  }
  return cap*fmod(a*1.618,1);
}
#endif
