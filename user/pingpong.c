#include <stddef.h>
#include "kernel/types.h"
#include "user.h"

int main(int, char*[]) 
{
  int pipe_fd[2];
  int ret = pipe(pipe_fd);
  if (ret != 0) {
    exit(1);
  }
  int pid = fork();
  if (pid > 0) {
    printf("parent process %d\n", getpid());
    // parent process
    char flag = 'a';
    // write pipe
    write(pipe_fd[1], &flag, 1);
    close(pipe_fd[1]);

    // wait child exit and sync stdout
    wait(NULL);
    char buf;
    read(pipe_fd[0], &buf, 1);
    printf("%d: received pong\n", getpid());
    close(pipe_fd[0]);
    exit(0);
  } else if (pid == 0) {
    // child process
    printf("child process %d\n", getpid());
    char buf;
    read(pipe_fd[0], &buf, 1);
    printf("%d: received ping\n", getpid());
    close(pipe_fd[0]);

    write(pipe_fd[1], &buf, 1);
    close(pipe_fd[1]);
    exit(0);
  } else {
    // error
    exit(1);
  }
}
