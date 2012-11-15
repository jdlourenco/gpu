#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#define BUF_LENGTH 512

int copy_to_cache(char* file_path, char* original_path);

/*
 * copy file from src_path to dst_path
 *
 * returns 0 if sucessfull
 */
int copy_file(char* dst_path, char* src_path);


