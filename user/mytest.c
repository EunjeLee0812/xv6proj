#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "user.h"
#include "../kernel/fcntl.h"
#include "../kernel/memlayout.h"
#include "../kernel/param.h"
#include "../kernel/spinlock.h"
#include "../kernel/sleeplock.h"
#include "../kernel/fs.h"
#include "../kernel/memlayout.h"
#include "../kernel/syscall.h"

#ifndef PGSIZE
#define PGSIZE 4096
#endif

void show_freemem(char *msg) {
  int n = freemem();
  printf("%s: free pages = %d\n", msg, n);
}
void test_anonymous() {
  int len = 2*PGSIZE;
  show_freemem("Before anon mmap");

  char *a1 = (char*)mmap(0, len, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_POPULATE, -1, 0);
  printf("MAP_ANONYMOUS|MAP_POPULATE addr: %p\n", a1);
  show_freemem("After anon populate");

  munmap((uint64)a1);
  show_freemem("After anon unmap");

  char *a2 = (char*)mmap(0, len, PROT_READ|PROT_WRITE, MAP_ANONYMOUS, -1, 0);
  printf("MAP_ANONYMOUS (lazy) addr: %p\n", a2);
  *a2 = 'X'; // lazy 할당 유도
  show_freemem("After lazy page fault");
  munmap((uint64)a2);
  show_freemem("After lazy unmap");
}
void test_filemap() {
  int fd = open("README", O_RDONLY);
  if(fd < 0) {
    printf("Cannot open README\n");
    exit(1);
  }

  show_freemem("Before file mmap");
  char *f1 = (char*)mmap(0, PGSIZE, PROT_READ, MAP_POPULATE, fd, 0);
  printf("file mmap populate: %p -> first char = %c\n", f1, f1[0]);
  show_freemem("After file populate");

  munmap((uint64)f1);
  show_freemem("After file unmap");

  char *f2 = (char*)mmap(0, PGSIZE, PROT_READ, 0, fd, 0);
  printf("file mmap lazy: %p -> first char = %c\n", f2, f2[0]);
  show_freemem("After file lazy");

  munmap((uint64)f2);
  close(fd);
}
void test_fork() {
  int fd = open("README", O_RDONLY);
  char *m = (char*)mmap(0, PGSIZE, PROT_READ, MAP_POPULATE, fd, 0);
  printf("Before fork: %c\n", m[0]);

  int pid = fork();
  if(pid == 0){
    printf("Child sees: %c\n", m[0]);
    munmap((uint64)m);
    exit(0);
  } else {
    wait(0);
    printf("Parent still sees: %c\n", m[0]);
    munmap((uint64)m);
  }
  close(fd);
}
int main() {
  printf("===== mmap system call test =====\n");
  test_anonymous();
  test_filemap();
  test_fork();
  exit(0);
}

