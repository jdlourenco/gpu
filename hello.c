/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall `pkg-config fuse --cflags --libs` hello.c -o hello
*/

#define FUSE_USE_VERSION 26
#define PASS "797979797979797979797979797979797979797979797979797979"

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <dirent.h>

#include "cache_util.h"
#include "util.h"

/*
int copy_file(char *source, char *dest);
int copy_to_cache(char* file_path, char* original_path);
*/
cache* file_cache = NULL;

static const char* cache_dir = "/tmp/cudaram";
static const char* backing_dir = "/home/ze/back";

char* extract_filename(const char* path);

static int hello_getattr(const char *path, struct stat *stbuf) {

  int res;
  char* back_path;

  printf("FUSE: call to getattr of %s\n",path);
  
  back_path = (char*) malloc(sizeof(char) * (strlen(backing_dir)+strlen(path)+1));
  strcpy(back_path,backing_dir);
  strcat(back_path,path);
  
  res = lstat(back_path, stbuf);

  if (res == -1)
    return -errno;
  
  return res;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
  
  DIR *dp;
  struct dirent *de;

  char* back_path = malloc(sizeof(char) * (strlen(backing_dir) + strlen(path) + 1));
  strcpy(back_path,backing_dir);
  strcat(back_path,path);

  printf("FUSE: call to readdir %s will ls %s\n",path,back_path);
  
  dp = opendir(back_path);

  if(dp==NULL)
    return -errno;

  while((de=readdir(dp)) != NULL) {
    struct stat st;
    memset(&st, 0, sizeof(st));
    st.st_ino = de->d_ino;
    st.st_mode = de->d_type << 12;
    if (filler(buf, de->d_name, &st, 0))
      break;
  }

  closedir(dp);
  return 0;
}


//so vai funcionar para ficheiros sem hierarquia
static int hello_open(const char *path, struct fuse_file_info *fi) {

  int fd;

  char* back_path = malloc(sizeof(char) * (strlen(backing_dir) + strlen(path) + 1));
  char* cache_path;

  strcpy(back_path,backing_dir);
  strcat(back_path,path);

  printf("FUSE: call to open %s\ncache:",path);
  show_cache_entries(file_cache);

  if(contains_entry(file_cache,(char *) path))
    printf("File in cache\n");
  else {
    printf("File not in cache\n");
    if(copy_file(back_path, (char*)path))
      printf("copy to cache failed\n");
  }

  cache_path = malloc(sizeof(char) * (strlen(cache_dir) + strlen(path) + 1));
  strcpy(cache_path,cache_dir);
  strcat(cache_path,path);

  fd = open(back_path, fi->flags);
  fi->fh = fd;
  
  return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {

  int res;
  /*  char* filename = extract_filename(path);
  char* back_path = malloc(sizeof(char) * (strlen(backing_dir) + 1 + strlen(filename) + 1));

  strcpy(back_path,backing_dir);
  strcat(back_path,"/");
  strcat(back_path,filename);
  
  printf("FUSE: call to read %s on %s\n",filename,path);
  printf("FUSE: will open %s\n",back_path);*/
  
  /*  (void) fi;
  fd = open(back_path, O_RDONLY);
  if (fd == -1)
    return -errno;
  */
  res = pread(fi->fh, buf, size, offset);
  if (res == -1)
    res = -errno;
  
  close(fi->fh);
  
  return res;
}

static int hello_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {

  /*  char* filename = extract_filename(path);
  char* back_path = malloc(sizeof(char) * (strlen(backing_dir) + 1 + strlen(filename) + 1));

  strcpy(back_path,backing_dir);
  strcat(back_path,"/");
  strcat(back_path,filename);
  
  printf("FUSE: call to write %s\n",filename);
  printf("FUSE: will write to %s\n",back_path);

  int fd;*/
  int res;

  /*  (void) fi;
  fd = open(back_path, O_WRONLY);
  if (fd == -1)
    return -errno;

  printf("chegou aqui\n");

  printf("FUSE: write:\n");*/

  res = pwrite(fi->fh, buf, size, offset);
  if (res == -1)
    res = -errno;

  close(fi->fh);
  return res;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
  int res;

  /* On Linux this could just be 'mknod(path, mode, rdev)' but this
     is more portable */
  if (S_ISREG(mode)) {
    res = open(path, O_CREAT | O_EXCL | O_WRONLY, mode);
    if (res >= 0)
      res = close(res);
  } else if (S_ISFIFO(mode))
    res = mkfifo(path, mode);
  else
    res = mknod(path, mode, rdev);
  if (res == -1)
    return -errno;

  return 0;
}

static int xmp_chmod(const char *path, mode_t mode)
{
  int res;

  res = chmod(path, mode);
  if (res == -1)
    return -errno;

  return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid)
{
  int res;

  res = lchown(path, uid, gid);
  if (res == -1)
    return -errno;

  return 0;
}

static int xmp_utimens(const char *path, const struct timespec ts[2])
{
  int res;
  struct timeval tv[2];

  tv[0].tv_sec = ts[0].tv_sec;
  tv[0].tv_usec = ts[0].tv_nsec / 1000;
  tv[1].tv_sec = ts[1].tv_sec;
  tv[1].tv_usec = ts[1].tv_nsec / 1000;

  res = utimes(path, tv);
  if (res == -1)
    return -errno;

  return 0;
}

static int xmp_truncate(const char *path, off_t size) {
  int res;

  char* filename = extract_filename(path);
  char* back_path = malloc(sizeof(char) * (strlen(backing_dir) + 1 + strlen(filename) + 1));

  strcpy(back_path,backing_dir);
  strcat(back_path,"/");
  strcat(back_path,filename);
  
  printf("FUSE: call to truncate %s\n",filename);
  printf("FUSE: will truncate %s to %jd bytes\n",back_path,size);

  res = truncate(back_path, size);
  if (res == -1)
    return -errno;

  return 0;
}

static int xmp_mkdir(const char *path, mode_t mode) {

  char* back_path = malloc(sizeof(char) * (strlen(backing_dir) + strlen(path) + 1));
  strcpy(back_path,backing_dir);
  strcat(back_path,path);

  printf("FUSE: call to mkdir %s\n",path);
  printf("FUSE: will create dir on %s\n",back_path);
  
  if(mkdir(back_path, mode))
    return -1;
  
  return 0;
}


static struct fuse_operations hello_oper = {
  .getattr	= hello_getattr,
  .readdir	= hello_readdir,
  .mknod	= xmp_mknod,
  .open		= hello_open,
  .read		= hello_read,
  .write	= hello_write,
  .chmod        = xmp_chmod,
  .chown        = xmp_chown,
  .utimens      = xmp_utimens,
  .truncate	= xmp_truncate,
  .mkdir        = xmp_mkdir
};


/* isto deve tar mal
 * nao quero so o filename acho eu
 * quero tudo
 *
 */
char* extract_filename(const char* path) {

  char* pathname = malloc(sizeof(char)*(strlen(path)+1));
  strcpy(pathname,path);

  const char* delim = "//";
  char* result;
  char* filename;
  char* aux;
  
  filename = strtok(pathname,delim);
  
  while((aux = strtok(NULL,delim)) != NULL)
    filename = aux;

  result = malloc(sizeof(char) * strlen(filename)+1);
  strcpy(result,filename);
  
  return result;
}



int main(int argc, char *argv[]) {
  cache* file_cache = NULL;

  /*
   * should launch thread here that maintains consistency
   * between cache and disk storage
   *
   */
  
  return fuse_main(argc, argv, &hello_oper, &file_cache);
}

