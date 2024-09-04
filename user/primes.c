#include <stddef.h>
#include "kernel/types.h"
#include "user.h"

// key: when child copy fd table, what status it is.

__attribute__((noreturn))
  static void sieve(int read_fd) {
    int prime;
    // the first number from pipe must be prime 
    int n = read(read_fd, &prime, sizeof(int));
    if (n <= 0) exit(0);

    // must sequence, in this time point, not call fork()
    printf("prime %d\n", prime);

    // create right_pipe for right_ps
    int pipe_next[2];
    int ret = pipe(pipe_next);
    if (ret != 0) {
      exit(1);
    }
    int pid = fork();
    if (pid == 0) {
      close(pipe_next[1]);
      sieve(pipe_next[0]);
    }
    close(pipe_next[0]);
    while (1) {
      int next_number;
      n = read(read_fd, &next_number, sizeof(int));
      if (n <= 0) break;
      if (next_number % prime == 0) continue;
      // send right_ps
      write(pipe_next[1], &next_number, sizeof(int));
    }
    close(read_fd);
    close(pipe_next[1]);
    wait(NULL);
    exit(0);
  }

int main(int, char*[]) 
{
  int pipe_fd[2];
  int ret = pipe(pipe_fd);
  if (ret != 0) {
    exit(1);
  }

  int pid = fork();
  if (pid == 0) {
    // first child process 
    close(pipe_fd[1]);
    sieve(pipe_fd[0]);
  }

  close(pipe_fd[0]);
  // main process send [2, 35]
  int i;
  for (i = 2; i <= 35; ++i) {
    write(pipe_fd[1], &i, sizeof(int));
  }
  close(pipe_fd[1]);

  wait(NULL);
  exit(0);
}

