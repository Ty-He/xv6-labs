#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

static char* command;

uint execlines(char** argv, uint last)
{

  if (last >= MAXARG) {
    printf("xargs: too many args.\n");
    return 0;
  }
  uint ps_size = 0;
  char line[128];
  uint n;
  while (1) {
    gets(line, sizeof(line));
    n = strlen(line);
    if (!n) break;
    line[n - 1] = 0;
    argv[last] = malloc(n);
    strcpy(argv[last], line);
    ps_size++;
    int pid = fork();
    if (pid == 0) {
      exec(command, argv);
    }
    free(argv[last]);
  }
  return ps_size;
}

// stdin -> command line
int main(int argc, char* args[]) 
{
  if (argc < 2) {
    printf("xargs: lack command.\n");
    exit(0);
  }
  char* argv[MAXARG];
  command = args[1];
  uint size = 0;
  for (int i = 1; i < argc; ++i) {
    argv[size++] = args[i];
  }
  uint s = execlines(argv, size);
  while (s--) wait(0);
  exit(0);
}

