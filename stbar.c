#include <stdio.h>
#include <string.h>
#include "stbar.h"
#include "terminal.h"
#include "text.h"
#include "msgbar.h"

StBar status_bar;

// 상태바 초기화
void s_stbar(const char *filename) {
    strncpy(status_bar.filename, filename, sizeof(status_bar.filename) - 1);
    status_bar.filename[sizeof(status_bar.filename) - 1] = '\0'; 
    status_bar.total_lines = 0;
    status_bar.cursor_row  = 0;
    status_bar.cursor_col = 0;
    co_li(filename); 
}

void d_stbar() {
    int rows, cols;
    terminal1(&rows, &cols);

    // 줄 수 및 커서 위치 동기화
    to_li();
    status_bar.cursor_row = cursor_row;
    status_bar.cursor_col = cursor_col;

    // 상태 바 출력
    printf("\033[%d;1H", rows - 2); // 상태 바 위치
    printf("\033[7m[%s] - %d Lines | Cursor: %d/%d\033[0m", 
           status_bar.filename, 
           status_bar.total_lines, 
           status_bar.cursor_row + 1, 
           status_bar.cursor_col + 1);
    printf("\n");
    fflush(stdout);

    // 커서 위치 복원
    printf("\033[%d;%dH", cursor_row + 1, cursor_col + 1);
    fflush(stdout);
}



void co_li(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("file open error");
        return;
    }

    char line[512];
    int line_count = 0;

    while (fgets(line, sizeof(line), file)) {
        line_count++;
        strcpy(text[line_count - 1], line);
    }
    
    status_bar.total_lines = line_count;
    fclose(file);
}

void up_cursor(int new_row, int new_col) {
    status_bar.cursor_row = new_row - 1;  // 1-based to 0-based index 변환
    status_bar.cursor_col = new_col - 1;  // 1-based to 0-based index 변환
    d_stbar();  // 상태 바 갱신
}


// 현재 텍스트의 줄 수를 계산하여 반환
void to_li() {
    int line_count = 0;
    for (int i = 0; i < MAX_LINES; i++) {
        if (text[i][0] != '\0') { 
            line_count++;
        }
    }
    status_bar.total_lines = line_count; // 줄 수를 상태 바에 반영
}
