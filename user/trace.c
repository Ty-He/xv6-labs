#include "kernel/types.h"
#include "user/user.h"

__attribute__((noreturn))
void useless_child()
{
  if (fork() == 0) {
    if (fork() == 0) {
      if (fork() == 0) exit(0);
      wait(0);
      exit(0);
    } 
    wait(0);
    exit(0);
  }
  wait(0);
  exit(0);
}

// bad solution
int old_main(int argc, char* args[])
{
  printf("trace pid = %d\n", getpid());
  if (argc < 3) {
    printf("trace: lack args\n");
    exit(0);
  }

  int mask = atoi(args[1]);
  trace(mask);

  // if (fork() == 0) useless_child();
  // wait(0);

  // printf("mask = %d\n", mask);
  int pid = fork();
  if (pid == 0) {
    // printf("child ps: %s\n", args[2]);
    exec(args[2], &args[2]);
  }
  /*
  int status;
  int child = wait(&status);
  printf("child%d ret = %d\n", child, status);
  */
  wait(0);
  exit(0);

}

int main(int argc, char* args[]) 
{
  // printf("trace pid = %d\n", getpid());
  if (argc < 3) {
    printf("trace: lack args\n");
    exit(0);
  }

  int mask = atoi(args[1]);
  trace(mask);

  exec(args[2], &args[2]);
  return 0;
}
