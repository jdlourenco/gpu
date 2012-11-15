#include "util.h"

/*
 * copy file from src_path to dst_path
 *
 * returns 0 if sucessfull
 */
int copy_file(char* dst_path, char* src_path) {
  
  int src_fd, dst_fd;
  char *buf;
  int n_read;
  struct stat source_stat;
  
  
  if((src_fd = open(src_path,O_RDONLY)) < 0) {
    printf("unable to open source file\n");
    return -1;
  }

  fstat(src_fd,&source_stat);

  /*  if((dst_fd = open(dst_path,O_CREAT|O_WRONLY,S_IRUSR|S_IWUSR)) < 0) {*/
  if((dst_fd = open(dst_path,O_CREAT|O_WRONLY,source_stat.st_mode)) < 0) {
    printf("unable to open destination file\n");
    return -1;
  }

  buf = malloc(sizeof(char) * BUF_LENGTH);
  
  while((n_read = read(src_fd,buf,BUF_LENGTH)) > 0) {
    //    printf("read %d bytes:%s\n",n_read,buf);
    write(dst_fd,buf,n_read);
  }
  
  futimes(src_fd,NULL);

  close(src_fd);
  close(dst_fd);
  free(buf);
  return 0;
}

/*
int main() {

  char* source = "/tmp/test/test";
  char* destination = "/tmp/test/test_copy";
  
  printf("will copy %s to %s\n",source,destination);
  if(copy_file(destination,source))
    printf("copy was not sucessful\n");

  printf("copy was sucessful\n");
  
  return 0;
}
*/
