
#include <stdio.h>
#include <string.h>
#include "hmap.h"

#define F_BUFF_SZ 256  /* size of read buffer */
#define INIT_CAP  10   /* number of initial bucket entries */
#define BRK_CHR   " ,;.?!\"()[]\r\n"

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
    fprintf(stderr, "USAGE: hash <key> <file_0> [... <file_N>]\n");
    return -1;
  }
  hash_map_free(&h_map);

  return r_val;
}
