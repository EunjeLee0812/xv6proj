#include "../kernel/types.h" // uint 타입 정의를 위해 필요합니다.
#include "user.h"           // printf, meminfo, exit 함수 선언을 위해 필요합니다.

int main(void) {
    int free_memory_bytes;
    
    // meminfo 시스템 콜을 호출하여 사용 가능한 메모리 크기(바이트)를 가져옵니다.
    free_memory_bytes = meminfo();

    if (free_memory_bytes < 0) {
        // 시스템 콜 실패 시 (보통 -1 반환)
        printf("Error: meminfo system call failed.\n");
        exit(0);
    }
    
    // 결과 출력 (xv6의 printf는 파일 디스크립터 1을 첫 번째 인수로 받습니다.)
    printf("Available free memory: %d bytes\n", free_memory_bytes);

    // 추가 테스트: 메모리 사용 전후 비교 (선택 사항)
    
    int *p = (int*)malloc(10 * 4096); // 10 페이지 할당
    if (p) {
        int after_alloc = meminfo();
        printf("Memory after allocation: %d bytes (Difference: %d bytes)\n", 
               after_alloc, free_memory_bytes - after_alloc);
        free(p);
    }
    

    exit(0);
}
