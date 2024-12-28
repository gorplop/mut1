#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "mut1.h"


int main(int argc, char** argv){
  char* filename;
  struct stat file_stat;
  uint8_t *elf;
  Elf64_Ehdr *ehdr;
  char* mutfile;
  int mutfile_fd;
  
  if(argc < 2) {
    printf("%s: No file given\n", argv[0]);
    printf("Usage: %s <elf file>\n", argv[0]);
    return 1;
  }
  filename = argv[1];
  
  int fd = open(filename, O_RDONLY);
  if(-1 == fd) {
    perror("open");
    return -1;
  }

  // Load up the whole file into memory
  fstat(fd, &file_stat);
  DEBUG("filesz = %d\n", file_stat.st_size);

  elf = mmap(NULL, file_stat.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);  
  DEBUG("elf location = %p\n", elf);
  ehdr = (Elf64_Ehdr*) elf;
  
  if(ehdr->e_machine != 0xb7){
    fprintf(stderr, "Not aarch64.\n");
    return 1;
  }
  
  srand(time(NULL));
  mutate_xrefs(elf);

  //Dump new file to disk under new name
  mutfile = malloc(strlen(filename) + 5); //".mut"+NUL
  strcpy(mutfile, filename);
  strcpy(mutfile+strlen(filename), ".mut");
  printf("Will write to %s\n", mutfile);
  if(!mutfile) {
    DEBUG("malloc fail!\n"); return 1;
  }
  
  mutfile_fd = open(mutfile, O_RDWR | O_CREAT, 0600);
  if(-1 == mutfile_fd) {
    perror("mutfile open fail");
  }
  write(mutfile_fd, elf, file_stat.st_size);

  DEBUG("copy permissions 0%o.. ", file_stat.st_mode);
  if(-1 == fchmod(mutfile_fd, file_stat.st_mode)){
    perror("fchmod fail!\n");
  }
  close(fd);
  close(mutfile_fd);
  printf("OK\n");
  return 0;

}
