#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#define __(x)    #x
#define ERROR(x) { fprintf(stderr, "ERROR" __(x) "\n"); exit(x); }

void _start() {
  int fd;
  int n;
  // If O_CREAT then have to provide mode
  if((fd = open("/tmp/x", O_RDWR | O_CREAT, S_IRWXU)) == -1){
    ERROR(1);
  }
  n = write(fd, "Hello, aarch64!\n", 16);
  if(n != 16) ERROR(2);
  exit(0);
}
