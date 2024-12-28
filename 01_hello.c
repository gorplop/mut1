#include <unistd.h>
#include <stdlib.h>

void _start() {
  const char msg[] = "Hello, ARM!\n";
  write(0, msg, sizeof(msg));
  exit(0);
}
