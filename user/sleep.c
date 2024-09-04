#include "kernel/types.h"
#include "user.h" // system call

#define LACK_ARGS "lack sleep duration!\n"
#define BAD_ARGS "bad args, should be [sleep n]"

int main(int argc, char* args[]) 
{
  if (argc <= 1) {
    write(1, LACK_ARGS, sizeof(LACK_ARGS));
    exit(0);
  } else if (argc > 2) {
    write(1, BAD_ARGS, sizeof(BAD_ARGS));
    exit(0);
  } 

  int n = atoi(args[1]);
  sleep(n);
  exit(0);
}
