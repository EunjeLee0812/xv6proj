// mytest.c — 대규모 페이지테이블 분기 생성/검증용
#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "user.h"
#include "../kernel/fcntl.h"
#include "../kernel/memlayout.h"

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

// 4KB(=4096) 페이지를 mmap 해보자 (총 16MiB)
#define NPAGES     4096
#define BYTES_TOTAL ((uint64)NPAGES * PGSIZE)

// SV39 기준: L0(leaf) 1 PTP가 2MiB(=512 * 4KiB)를 커버
//             L1 1 PTP는 1GiB 밑의 L0 테이블들을 가리킴
#define CHUNK_2MB  ((uint64)512 * PGSIZE)
#define CHUNK_1GB  ((uint64)1 << 30)

static void show_free(const char *msg) {
  int n = freemem();
  printf("%s: free pages = %d\n", msg, n);
}

static uint64 ceil_div_u64(uint64 a, uint64 b) {
  return (a + b - 1) / b;
}

void test_mass_pt_fanout(void) {
  int before = freemem();
  show_free("before mmap");

  // 큰 연속 구간 할당 (익명 + populate → 실제 물리페이지 잡힘)
  char *base = (char*)mmap(0, BYTES_TOTAL,
                           PROT_READ | PROT_WRITE,
                           MAP_ANONYMOUS | MAP_POPULATE,
                           -1, 0);
  if ((uint64)base == (uint64)-1 || base == 0) {
    printf("mmap failed\n");
    exit(1);
  }
  int after_map = freemem();

  printf("mapped %d pages (%d bytes) at %p\n", NPAGES, (int)BYTES_TOTAL, base);

  // 예상되는 페이지테이블 수(대략): L0 = (커버한 2MiB 블록 수), L1 = (커버한 1GiB 블록 수)
  uint64 a = (uint64)base;
  uint64 off2m = a % CHUNK_2MB;
  uint64 off1g = a % CHUNK_1GB;

  uint64 nb2m = ceil_div_u64(off2m + BYTES_TOTAL, CHUNK_2MB); // L0 PTP 개수 예상
  uint64 nb1g = ceil_div_u64(off1g + BYTES_TOTAL, CHUNK_1GB); // L1 PTP 개수 예상
  printf("expected new PT pages ~= L0:%d + L1:%d = %d  (root 제외, 대략치)\n",
         (int)nb2m, (int)nb1g, (int)(nb2m + nb1g));

  // 2MiB 블록 경계 목록을 출력(실제 여러 '분기'가 생기는 위치 가늠용)
  printf("2MB blocks covered: %d\n", (int)nb2m);
  for (int i = 0; i < (int)nb2m; i++) {
    // base가 2MiB 정렬이 아닐 수 있으므로, base 기준으로 위쪽 2MiB 경계들을 나열
    uint64 first_block = (a / CHUNK_2MB) * CHUNK_2MB;
    uint64 addr = first_block + (uint64)i * CHUNK_2MB;
    if (addr < a) addr += CHUNK_2MB;
    if (addr >= a + BYTES_TOTAL) break;
    printf("  block[%d]: %p .. %p\n", i, (void*)addr, (void*)(addr + CHUNK_2MB - 1));
  }

  // 혹시 구현이 lazy일 수도 있으니 페이지마다 1바이트씩 터치
  for (int i = 0; i < NPAGES; i++) {
    base[i * PGSIZE] = (char)(i & 0xFF);
  }
  int after_touch = freemem();

  int delta_map   = before - after_map;
  int delta_touch = after_map - after_touch;

  printf("deltas: map=%d pages, touch=%d pages (total drop=%d)\n",
         delta_map, delta_touch, before - after_touch);
  printf("note) 이상적이라면 data=%d + PT≈%d 만큼 줄어들 수 있음\n",
         NPAGES, (int)(nb2m + nb1g));

  // 해제 및 회수 확인
  if (munmap((uint64)base) < 0) {
    printf("munmap failed\n");
    exit(1);
  }
  int after_unmap = freemem();
  printf("after unmap: free pages = %d, recovered=%d\n",
         after_unmap, after_unmap - after_touch);
}

int main(void) {
  printf("===== massive pagetable fan-out test =====\n");
  test_mass_pt_fanout();
  exit(0);
}

