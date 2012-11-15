//#include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct cache_entry_t {
  char* filename;
  struct cache_entry_t* next;
} cache_entry;

typedef struct cache_t {
  cache_entry* file_cache;
  signed short int cache_size;
} cache;

void cache_insert(cache* cache, char* filename);
void cache_remove(cache* cache, char* filename);
int contains_entry(cache* cache, char* filename);
void show_cache_entries(cache* cache);

int write_cache_updates(cache* cache);
