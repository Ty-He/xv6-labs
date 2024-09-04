// #include <stddef.h>
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "user/user.h"

struct filelist {
  char** container;
  uint size;
  uint cap;
};

typedef struct filelist list;

list* get_list(int cap) 
{
  list* ls = malloc(sizeof(list));
  if (!ls) {
    printf("get_list: malloc error\n");
    exit(1);
  }    
  ls->size = 0;
  ls->cap = cap;
  ls->container = malloc(sizeof(char*) * ls->cap);
  if (!ls->container) {
    printf("get_list: malloc error\n");
    exit(1);
  }
  return ls;
}

// do not free node
void destory(list* ls) {
  free(ls->container);
  free(ls);
}

list* append(list* ls, char* node) {
  if (ls->size == ls->cap) {
    list* new_ls = get_list(ls->cap * 2);
    new_ls->size = ls->size;
    // should not use memcpy(): arg not sample
    memcpy(new_ls->container, ls->container, ls->size * sizeof(char*));
    // for (int i = 0; i < ls->size; ++i) 
    //    new_ls->container[i] = ls->container[i];
    destory(ls);
    return append(new_ls, node);
  } else {
    ls->container[ls->size] = node;
    ls->size++;
    return ls;
  }
}



void find(char const* path, char const* filename)
{
  // printf("[find]: %s\n", path);

  int fd;
  if ((fd = open(path, O_RDONLY)) < 0) {
    printf("find: cannot open\n");
    return;
  }

  struct stat st;
  if ((fstat(fd, &st)) < 0) {
    printf("find: cannot fstat\n");
    close(fd);
    return;
  }

  if (st.type != T_DIR) {
    // printf("find: path should be dir\n");
    close(fd);
    return;
  }

  struct dirent de;
  list* ls = get_list(5);
  while (read(fd, &de, sizeof(de))) {
    if (de.inum == 0) continue;
    if (!strcmp(de.name, ".") || !strcmp(de.name, "..")) 
      continue;

    int m = strlen(path), n = strlen(de.name);
    uint len = m + n + 1;
    char* new_path = malloc(len + 1);
    if (!new_path) {
      printf("find: malloc error\n");
      exit(1);
    }
    memcpy(new_path, path, m);
    new_path[m] = '/';
    memcpy(new_path + m + 1, de.name, n);
    new_path[len] = '\0';

    if (!strcmp(de.name, filename)) 
      printf("%s\n", new_path);
    ls = append(ls, new_path);
  }
  close(fd);

  // printf("size = %d, cap = %d\n", ls->size, ls->cap);
  for (int i = 0; i < ls->size; ++i) {
    // printf("%d, %p\n", i, ls->container[i]);
    find(ls->container[i], filename);
    free(ls->container[i]);
  }
  destory(ls);
}

int main(int argc, char* args[]) 
{
  if (argc != 3) {
    printf("find: bad args [find path filename]\n");
  } else {
    int n = strlen(args[1]);
    if (args[1][n - 1] == '/') 
      args[1][n - 1] = '\0';
    find(args[1], args[2]);
  }
  exit(0);
}
