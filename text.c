
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "text.h"
#include "terminal.h"
#include "stbar.h"
#include "msgbar.h"

#define MAX_LINES 1000
#define MAX_COLS  80

char text[MAX_LINES][MAX_COLS];
int cursor_row = 0;  // 커서 행
int cursor_col = 0;  //  커서 열

void display() {
    #ifdef _WIN32
        system("cls"); // Windows에서 화면 지우기
    #else
        printf("\033[H\033[J"); // 유닉스/리눅스에서 화면 지우기
    #endif

    // 텍스트 출력
    for (int i = 0; i < MAX_LINES && text[i][0] != '\0'; i++) {
        printf("%s\n", text[i]);
    }
      // 상태 바와 메시지 바 출력 위치로 이동
    d_stbar(); 
    d_msgbar(); 

    fflush(stdout);
}


void a_lines() {
    if (cursor_row >= MAX_LINES - 1) {
        return; // 최대 줄 개수를 초과하지 않도록 제한
    }

    // 다음 줄로 이동할 내용을 복사
    char temp[MAX_COLS];
    strncpy(temp, &text[cursor_row][cursor_col], MAX_COLS - 1);
    temp[MAX_COLS - 1] = '\0'; 

    // 현재 줄의 커서 이후 내용을 삭제
    text[cursor_row][cursor_col] = '\0';

    // 현재 줄 아래로 내용을 밀기
    for (int i = MAX_LINES - 1; i > cursor_row + 1; i--) {
        strncpy(text[i], text[i - 1], MAX_COLS);
    }

    // 다음 줄에 이동한 내용을 설정
    strncpy(text[cursor_row + 1], temp, MAX_COLS - 1);
    text[cursor_row + 1][MAX_COLS - 1] = '\0';

    // 커서 위치를 다음 줄로 이동
    cursor_row++;
    cursor_col = 0;
}



void insert(char ch) {
    if (cursor_row < MAX_LINES && cursor_col < MAX_COLS - 1) {
        // 현재 커서 위치에 문자 삽입
        memmove(&text[cursor_row][cursor_col + 1],
                &text[cursor_row][cursor_col],
                strlen(&text[cursor_row][cursor_col]) + 1);
        text[cursor_row][cursor_col] = ch;
        cursor_col++;
    } else if (cursor_col >= MAX_COLS - 1 && cursor_row < MAX_LINES - 1) {
        // 줄 끝에서 다음 줄로 이동하며 문자 삽입
        a_lines();
        text[cursor_row][cursor_col] = ch;
        cursor_col++;
    }

    display();
    d_stbar();
    d_msgbar();
    printf("\033[%d;%dH", cursor_row + 1, cursor_col + 1);
    fflush(stdout);
}



void backspace() {
    if (cursor_col > 0) {
        //  문자 삭제 후 나머지 문자열 이동
        memmove(&text[cursor_row][cursor_col - 1], 
                &text[cursor_row][cursor_col], 
                strlen(&text[cursor_row][cursor_col]) + 1);
        cursor_col--; 
    } else if (cursor_row > 0) {
        //  이전 줄과 병합
        strcat(text[cursor_row - 1], text[cursor_row]);  // 현재 줄을 이전 줄 끝에 병합

        // 현재 줄 삭제 및 나머지 줄 이동
        for (int i = cursor_row; i < MAX_LINES - 1; i++) {
            strcpy(text[i], text[i + 1]);
        }
        text[MAX_LINES - 1][0] = '\0';  // 마지막 줄 초기화

        // 커서와 줄 개수 업데이트
        cursor_row--;  
        cursor_col = strlen(text[cursor_row]);  // 병합된 줄의 끝으로 이동
        input_row--; 
    } else {
        return;
    }

   
    display();
    d_stbar();
    printf("\033[%d;%dH", cursor_row + 1, cursor_col + 1);  
    fflush(stdout);
}



void home() {
    cursor_col = 0;
    printf("\033[%d;%dH", cursor_row + 1, cursor_col + 1); 
    fflush(stdout);
    d_stbar();
}


void end() {
    cursor_col = strlen(text[cursor_row]);
    printf("\033[%d;%dH", cursor_row + 1, cursor_col + 1); 
    fflush(stdout);
    d_stbar();
}


void page_up() {
    cursor_row = 0;
    cursor_col = 0;
    printf("\033[%d;%dH", cursor_row + 1, cursor_col + 1); 
    fflush(stdout);
    d_stbar();
}


void page_down() {
    for (int i = MAX_LINES - 1; i >= 0; i--) {
        if (text[i][0] != '\0') {
            cursor_row = i;
            cursor_col = strlen(text[cursor_row]);
            break;
        }
    }
    printf("\033[%d;%dH", cursor_row + 1, cursor_col + 1); 
    fflush(stdout);
    d_stbar();
}

void up() {
    if (cursor_row > 0) {
        cursor_row--;
        if (cursor_col > strlen(text[cursor_row])) {
            cursor_col = strlen(text[cursor_row]);
        }
        display(); 
        d_stbar();      
        d_msgbar();     

        
        printf("\033[%d;%dH", cursor_row + 1, cursor_col + 1);
        fflush(stdout);
    }
}

void down() {
    if (cursor_row < MAX_LINES - 1 && text[cursor_row + 1][0] != '\0') {
        cursor_row++;
        if (cursor_col > strlen(text[cursor_row])) {
            cursor_col = strlen(text[cursor_row]);
        }
        display(); 
        d_stbar();      
        d_msgbar();     

        
        printf("\033[%d;%dH", cursor_row + 1, cursor_col + 1);
        fflush(stdout);
    }
}


void left() {
    if (cursor_col > 0) {
        cursor_col--;
    } else if (cursor_row > 0) {
        cursor_row--;
        cursor_col = strlen(text[cursor_row]);
    }
    printf("\033[%d;%dH", cursor_row + 1, cursor_col + 1); 
    fflush(stdout);
    display(); 
    d_stbar();      
    d_msgbar();     
}

void right() {
    if (cursor_col < strlen(text[cursor_row])) {
        cursor_col++;
    } else if (cursor_row < MAX_LINES - 1 && text[cursor_row + 1][0] != '\0') {
        cursor_row++;
        cursor_col = 0;
    }
    printf("\033[%d;%dH", cursor_row + 1, cursor_col + 1); 
    fflush(stdout);
    display(); 
    d_stbar();      
    d_msgbar();     
}
