#include "../kernel/types.h" // <-- types.h를 가장 먼저 포함하여 'uint'를 정의합니다.
#include "user.h"           // <-- 그 다음 user.h를 포함하여 uint를 사용하는 함수 선언을 처리합니다.
// #include "../kernel/stat.h" // 불필요하여 제거

int main()
{
    int i;
    for(i=1;i<11;i++){
        // 이 부분은 printf 인자 에러가 이미 사라졌음을 가정하고 이전 코드를 유지합니다.
        printf("%d : ", i);
        int nice = getnice(i);
        if(nice == -1) {
            printf("Wrong PID\n");
        } else {       
	       	setnice(i, i);
       		int nice = getnice(i);
            	printf("nice -> %d",nice);
	    // getnice가 성공했을 때 출력할 내용이 필요하면 여기에 추가 (선택 사항)
        }
        printf("\n");
    }
    
    // exit()에 인수 1개를 넣어주어야 합니다!
    exit(0); // <-- 'user/user.h'의 선언(int exit(int))에 맞게 수정
}
