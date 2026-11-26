#include <stdio.h>
#include <string.h>
#include "msgbar.h"
#include "terminal.h" 
#include "text.h"
#include "stbar.h"

#define DEFAULT_MESSAGE "HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find"

typedef struct {
    char message[256];
} MsgBar;

MsgBar msg_bar;

// 메시지 바 초기화
void s_msgbar() {
    strncpy(msg_bar.message, DEFAULT_MESSAGE, sizeof(msg_bar.message) - 1);
    msg_bar.message[sizeof(msg_bar.message) - 1] = '\0'; 
    d_msgbar();  
}

void d_msgbar() {
    int rows, cols;
    terminal1(&rows, &cols);
    printf("\033[%d;1H", rows - 1); // 메시지 바를 마지막 줄로 이동
    printf("%s", msg_bar.message);
    fflush(stdout);
}

void up_msgbar(const char *new_message) {
    int rows, cols;
    terminal1(&rows, &cols);
    
    // 메시지 바를 지우고 새로운 메시지 출력
    printf("\033[%d;1H\033[K", rows); // 메시지 바가 있는 줄을 지우기
    printf("%s", new_message);
    fflush(stdout);
}
