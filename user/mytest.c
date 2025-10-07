#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "user.h"
#include "../kernel/fcntl.h"

// 총 CPU 점유 시간을 늘리기 위한 반복 횟수
#define LONG_LOOP_COUNT 50000000 

// CPU를 점유하는 함수 (긴 계산 루프)
void cpu_hog() {
    volatile int k = 0; // 최적화 방지
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < LONG_LOOP_COUNT; j++) {
            k++;
        }
    }
}

void
main(int argc, char *argv[])
{
    int pid;
    // 테스트 케이스: 최우선(0), 기본(20), 최하위(39)
    int nice_values[] = {0, 20, 39}; 
    char *names[] = {"T_High", "T_Base", "T_Low"};
    int num_processes = sizeof(nice_values) / sizeof(nice_values[0]);
    int i;

    printf("=== EE VDF Fairness Test START ===\n");
    printf("Expected: T_High(nice=0) uses most CPU; T_Low(nice=39) vruntime increases fastest.\n");

    for (i = 0; i < num_processes; i++) {
        pid = fork();

        if (pid < 0) {
            fprintf(2, "mytest: fork failed\n");
            break;
        }

        if (pid == 0) {
            // 자식 프로세스
            
            // setnice 시스템 콜 호출 (이 시스템 콜이 구현되어 있어야 함)
            if (setnice(getpid(), nice_values[i]) < 0) {
                fprintf(2, "Test %s: setnice failed. Check setnice implementation.\n", names[i]);
            }
            
            printf("Process %s (PID %d) starting with nice=%d...\n", 
                   names[i], getpid(), nice_values[i]);
            
            cpu_hog(); // CPU 점유 시작

            printf("Process %s (PID %d) finished.\n", names[i], getpid());
            exit(0);
        }
    }

    // 부모 프로세스는 모든 자식이 끝날 때까지 대기
    for (i = 0; i < num_processes; i++) {
        wait(0);
    }

    printf("=== EE VDF Fairness Test END ===\n");
    exit(0);
}
