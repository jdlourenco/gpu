#include "cache_util.h"
#include "util.h"

static const char* cache_dir = "/tmp/test_cache";
static const char* backing_dir = "/tmp/test";

int write_cache_updates(cache* file_cache) {
  
  cache_entry* aux;
  char* back_path;
  char* cache_path;
  struct stat back_stat, cache_stat;
  
  /* can alternately use dirty flag on cache list and set it on every write*/
  for(aux=file_cache->file_cache;aux;aux=aux->next) {
    
    back_path = malloc(sizeof(char) * (strlen(backing_dir) + strlen(aux->filename) + 1));
    strcpy(back_path,backing_dir);
    strcat(back_path,aux->filename);
    
    cache_path = malloc(sizeof(char) * (strlen(cache_dir) + strlen(aux->filename) + 1));
    strcpy(cache_path,cache_dir);
    strcat(cache_path,aux->filename);

    if(stat(back_path,&back_stat)) {
      printf("%s doesn't exist\n", back_path);
      return -1;
    }

    if(stat(cache_path,&cache_stat)) {
      printf("%s doesn't exist\n", cache_path);
      return -1;
    }

    if(back_stat.st_mtime < cache_stat.st_mtime) {
      printf("%s: cached file has been modified and needs to be updated\n",aux->filename);
      /* should copy whoole file or only modifications*/
      if(remove(back_path)) {
	printf("unable to remove %s\n",back_path);
	return -1;
      }

      if(copy_file(back_path,cache_path)) {
	printf("unable to copy cached file %s\n",aux->filename);
	return -1;
      }
    }

    else
      printf("cached file hasn't been modified\n");

    free(back_path);
    free(cache_path);
  }

  return 0;
}


void* cache_loop(void *arg) {
  
  cache* file_cache = (cache*) arg;

  show_cache_entries(file_cache);
  
  while(1) {
    sleep(3);
    printf("I'm the thread of the world\n");
    write_cache_updates(file_cache);
  }
}

void cache_insert(cache* cache, char* filename) {
  cache_entry *temp,*new_entry;

  temp = cache->file_cache;

  new_entry = (cache_entry*) malloc(sizeof(cache_entry));
  new_entry->filename = (char*) malloc(sizeof(char) * (strlen(filename) + 1));
  strcpy(new_entry->filename,filename);
  new_entry->next = cache->file_cache;
  cache->file_cache = new_entry->next;
}

void show_cache_entries(cache* cache) {

  int i;
  cache_entry* entry = cache->file_cache;
  if(entry==NULL) {
    printf("The list is empty\n");
    return;
  }
  for(i=0;entry!=NULL;i++,entry=entry->next) {
    printf("%d: %s\n",i,entry->filename);
  }
}

void cache_remove(cache* cache, char* filename) {

  cache_entry *prev,*temp;
  temp = cache->file_cache;

  if(temp==NULL) {
    printf("The list is empty\n");
    return;
  }
  
  prev=temp;
  while(temp!=NULL) {
    if(strcmp(temp->filename,filename)==0) {
      if(temp==cache->file_cache) /* First Node case */
	cache->file_cache=temp->next; /* shifted the header node */
      else
	prev->next=temp->next;
      
      free(temp);
      return;
    }
    else {
      prev=temp;
      temp=temp->next; 
    }
  }
}

int contains_entry(cache* cache, char* filename) {

  cache_entry* entry = cache->file_cache;

  while(entry) {
    if(!strcmp(entry->filename,filename))
      return 1;
    entry=entry->next;
  }
  
  return 0;
}


/*
int _main() {
  
  cache* file_cache=NULL;
  pthread_t thread;

  cache_insert(&file_cache,"/test1");
  cache_insert(&file_cache,"/test2");

  pthread_create(&thread,NULL,cache_loop,(void*)file_cache);
  
  while(1);
  //  write_cache_updates(file_cache);

  return 0;
}
*/
