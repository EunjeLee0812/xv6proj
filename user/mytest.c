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
#ifndef PROT_READ
#define PROT_READ  1
#define PROT_WRITE 2
#endif
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS 1
#define MAP_POPULATE  2
#endif

void test_prot_ro_write_fault() {
  int fd = open("README", O_RDONLY);
  if (fd < 0) { printf("open README fail\n"); return; }

  char *ro = (char*)mmap(0, PGSIZE, PROT_READ, 0, fd, 0);
  printf("RO map @%p first=%c\n", ro, ro[0]);

  int pid = fork();
  if (pid == 0) {
    // 여기가 죽어야 정상
    ro[0] = 'Z';
    printf("ERROR: write to RO succeeded\n");
    exit(1);
  } else {
    int st = 0;
    wait(&st);
    // st==1이면 자식이 위의 ERROR 경로로 정상종료함 → 실패
    if (st == 1) printf("FAIL: RO write not blocked\n");
    else         printf("OK: RO write killed child (st=%d)\n", st);
  }

  munmap((uint64)ro);
  close(fd);
}
void test_unmap_reclaim() {
  int before = freemem();
  char *a = (char*)mmap(0, 2*PGSIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_POPULATE, -1, 0);
  int after_map = freemem();
  printf("reclaim: before=%d after_map=%d delta=%d\n", before, after_map, before - after_map);

  munmap((uint64)a);
  int after_unmap = freemem();
  printf("reclaim: after_unmap=%d recovered=%d\n", after_unmap, after_unmap - after_map);
}
void test_fd_close_after_mmap() {
  int fd = open("README", O_RDONLY);
  if (fd < 0) { printf("open README fail\n"); return; }
  char *m = (char*)mmap(0, PGSIZE, PROT_READ, 0, fd, 0);
  close(fd); // 바로 닫음. mmap이 filedup 해놨으면 OK
  printf("after close: m[0]=%c\n", m[0]); // 읽히면 성공
  munmap((uint64)m);
}

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

  printf("\n\n===== mmap system call test =====\n");
  // 네가 이미 만든 기존 테스트들 호출 후 아래 3개도 호출
  test_prot_ro_write_fault();
  test_unmap_reclaim();
  test_fd_close_after_mmap();
  exit(0);
}

