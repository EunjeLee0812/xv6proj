#include "../kernel/types.h"
#include "user.h"

// 목표: P3 -> P4 -> P5 생성 후, P4는 P5를 기다리고, P3는 P4를 기다립니다.

void child_two_logic(void) {
    // Child 2 (PID 5)의 로직
    printf("Child 2 (PID %d): Process created. Working for 20 ticks.\n", getpid());
    printf("Child 2 (PID %d): Work finished. Exiting now.\n", getpid());
    exit(0);
}

void child_one_logic(int parent_pid) {
    // Child 1 (PID 4)의 로직
    int child_two_pid;
    int wait_result;
    
    printf("Child 1 (PID %d): Created by Parent %d. Now forking Child 2...\n", getpid(), parent_pid);
    
    // 1. Child 2 (PID 5) 생성
    child_two_pid = fork();
    
    if (child_two_pid < 0) {
        printf("Error: Child 1 fork failed.\n");
        exit(0);
    } else if (child_two_pid == 0) {
        // Child 2 프로세스 (PID 5)
        child_two_logic();
        // Child 2가 종료되면 다음 라인은 실행되지 않음
    } else {
        // Child 1 (PID 4)
        
        // 2. waitpid(5) 호출
        wait_result = waitpid(child_two_pid);

        // 3. Child 2의 종료 확인
        if (wait_result == 0) {
            printf("Child 1 (PID %d): Child 2 (PID %d) exited successfully. Exiting now.\n", getpid(), child_two_pid);
        } else {
            printf("Child 1 (PID %d): waitpid error for PID %d. Exiting now.\n", getpid(), child_two_pid);
        }
        exit(0);
    }
}

int main(void) {
    int child_one_pid;
    int wait_result;
    int parent_pid = getpid();
    
    printf("--------------------------------\n");
    printf("Parent (PID %d): Starting Cascading waitpid test.\n", parent_pid);

    // 1. Child 1 (PID 4) 생성
    child_one_pid = fork();

    if (child_one_pid < 0) {
        printf("Error: Parent fork failed.\n");
        exit(0);
    } 
    
    // ------------------------------------
    // [Child 1 Process (PID 4)]
    // ------------------------------------
    else if (child_one_pid == 0) {
        child_one_logic(parent_pid);
        // Child 1이 종료되면 다음 라인은 실행되지 않음
    } 
    
    // ------------------------------------
    // [Parent Process (PID 3)]
    // ------------------------------------
    else {
        // 2. waitpid(4) 호출
        wait_result = waitpid(child_one_pid); 

        // 3. Child 1의 종료 확인
        printf("Parent (PID %d): waitpid completed. Return Value: %d\n", parent_pid, wait_result);

        if (wait_result == 0) {
            printf("Parent (PID %d): Child 1 (PID %d) exited successfully. Test finished.\n", parent_pid, child_one_pid);
        } else {
            printf("Parent (PID %d): waitpid error for PID %d. Test finished.\n", parent_pid, child_one_pid);
        }
    }

    printf("--------------------------------\n");
    exit(0);
}
