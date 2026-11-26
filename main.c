#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "stbar.h"
#include "msgbar.h"
#include "text.h"
#include "terminal.h"

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
// 콘솔 모드 설정 함수
void set_console_mode() {
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    mode &= ~ENABLE_ECHO_INPUT;     
    mode &= ~ENABLE_LINE_INPUT;     // 줄 단위 입력을 비활성화
    SetConsoleMode(hStdin, mode);
}

#else
#ifdef __linux__
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

// Linux/macOS: 콘솔 모드 설정
void set_console_mode() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~(ICANON | ECHO); // 즉시 입력 및 에코 비활성화
    tcsetattr(STDIN_FILENO, TCSANOW, &t);

    system("stty -ixon");
}


int _getch() {
    struct termios oldt, newt;
    int ch;

    tcgetattr(STDIN_FILENO, &oldt);           // 현재 터미널 설정 가져오기
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);         // 즉시 입력과 에코 비활성화
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);  // 새로운 설정 적용
    ch = getchar();                           // 키 입력 읽기
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);  // 원래 설정 복구

    return ch;
}
#endif
#endif

char filename[256] = "no Name"; // 초기 파일명
bool is_modified = false;      // 파일 수정 여부 
int unsaved_exit_attempts = 0; // 저장하지 않은 상태로 종료하려는 시도 횟수
int input_row = 1;
bool is_search_mode = false;
bool is_searching = false; // 탐색 상태 
char search_query[256] = ""; // 검색어 저장
int search_row = -1; //  행
int search_col = -1; //  열


// 파일명 변경 및 텍스트 초기화
void filenamed(const char *new_name) {
    strncpy(filename, new_name, sizeof(filename) - 1);
    filename[sizeof(filename) - 1] = '\0';
    up_msgbar("Filename change.");
    s_stbar(filename);
    printf("\033[H\033[J");
    memset(text, 0, sizeof(text));
    cursor_row = 0;
    cursor_col = 0;
    input_row = 1;
    is_modified = false;
    d_stbar();
    d_msgbar();
}

// 파일 열기
void open_file(const char *file) {
    FILE *f = fopen(file, "r");
    if (!f) {
        up_msgbar("Error: no open file.");
        return;
    }
    filenamed(file);
    int line_num = 0;
    char line[MAX_COLS];
    while (fgets(line, sizeof(line), f) && line_num < MAX_LINES) {
        line[strcspn(line, "\n")] = '\0'; 
        strncpy(text[line_num], line, MAX_COLS - 1);
        text[line_num][MAX_COLS - 1] = '\0';
        line_num++;
    }
    fclose(f);
    cursor_row = line_num ;
    cursor_col = strlen(text[cursor_row]);
    input_row = line_num + 1;
    is_modified = false;
    up_msgbar("File open.");
    display();
    d_stbar();
}

// 파일 저장
void save_file(const char *file) {
    FILE *f = fopen(file, "w");
    if (!f) {
        up_msgbar("Error: no save file.");
        return;
    }
    for (int i = 0; i < MAX_LINES && text[i][0] != '\0'; i++) {
        fprintf(f, "%s\n", text[i]);
    }
    
    fclose(f);
    is_modified = false;
    char success_message[256];
    snprintf(success_message, sizeof(success_message), "File '%s' save.", file);
    up_msgbar(success_message);
}

void save_file2() {
    if (strcmp(filename, "no Name") == 0) {
        //  파일 이름 입력 요청 메시지 출력
        int rows, cols;
        terminal1(&rows, &cols);
        
        // 메시지와 입력을 같은 줄에서 처리
        printf("\033[%d;1H file name to save: ", rows); 
        fflush(stdout);

        char new_filename[256];
        if (scanf("%255s", new_filename) == 1) {
            strncpy(filename, new_filename, sizeof(filename) - 1);
            filename[sizeof(filename) - 1] = '\0';

            s_stbar(filename); // 파일 이름 변경 후 상태 바 업데이트
            save_file(filename); // 현재 텍스트를 파일에 저장

            
            display(); 
            d_stbar();      
            d_msgbar();     

            // 커서 초기화
            printf("\033[%d;%dH", cursor_row + 1, cursor_col + 1);
            fflush(stdout);
        } else {
            up_msgbar("input failed.");
        }

        // 파일 이름 입력 후 메시지 바와 텍스트 영역 복원
        display();
        d_stbar();
        d_msgbar();
    } else {
        save_file(filename); // 기존 파일 이름으로 저장
        display();      
        d_stbar();           
        d_msgbar();          
    }
}


// 종료 기능
void quit_program() {
    if (is_modified && unsaved_exit_attempts == 0) {
        up_msgbar("Press Ctrl-Q again.");
        unsaved_exit_attempts++;
    } else {
        printf("Exit.\n");
        exit(0);
    }
}

void start_search() {
    is_searching = true;
    search_query[0] = '\0';
    search_row = 0; // 검색 시작 위치를 문서의 첫 줄로 초기화
    search_col = 0;
    up_msgbar("Search: "); 
    display(); 
    d_stbar();
    d_msgbar();
}


void clear_highlight() {
    if (search_row >= 0 && search_col >= 0) {
        // 기존 하이라이트 줄 복원
        printf("\033[%d;1H%s", search_row + 1, text[search_row]);
        fflush(stdout);
    }
}

void highlight_search() {
    clear_highlight(); 

    if (search_row >= 0 && search_col >= 0) {
        int len = strlen(search_query); // 검색어 길이
        printf("\033[%d;1H", search_row + 1); // 해당 줄로 이동

        // 검색어 이전 텍스트 출력
        fwrite(text[search_row], 1, search_col, stdout);

        // 검색어 하이라이트
        printf("\033[7m%.*s\033[0m", len, &text[search_row][search_col]);

        // 검색어 이후 텍스트 출력
        printf("%s", &text[search_row][search_col + len]);

        fflush(stdout);
    }
}


bool search_text(const char *query, bool search_down) {
    clear_highlight(); 

    if (search_down) {
        // 현재 줄부터 마지막 줄까지 검색
        for (int i = search_row; i < MAX_LINES; i++) {
            char *start_pos = (i == search_row) ? &text[i][search_col + 1] : text[i];
            char *found = strstr(start_pos, query);
            if (found) {
                search_row = i;
                search_col = found - text[i];
                highlight_search();
                return true;
            }
        }

        // 처음부터 현재 줄까지 검색 (wrap-around)
        for (int i = 0; i <= search_row; i++) {
            char *found = strstr(text[i], query);
            if (found) {
                search_row = i;
                search_col = found - text[i];
                highlight_search();
                return true;
            }
        }
    } else {
        // 현재 줄부터 처음 줄까지 검색
        for (int i = search_row; i >= 0; i--) {
            char *found = strstr(text[i], query);
            char *last_valid_found = NULL;

            while (found) {
                // 현재 줄에서는 search_col 이전까지 유효
                if (i != search_row || found - text[i] < search_col) {
                    last_valid_found = found; // 유효한 마지막 검색 위치 저장
                }
                found = strstr(found + 1, query); // 다음 위치 검색
            }

            if (last_valid_found) {
                search_row = i;
                search_col = last_valid_found - text[i];
                highlight_search();
                return true;
            }
        }

        // 마지막 줄부터 현재 줄까지 검색 (wrap-around)
        for (int i = MAX_LINES - 1; i > search_row; i--) {
            char *found = strstr(text[i], query);
            char *last_valid_found = NULL;

            while (found) {
                last_valid_found = found; // 유효한 마지막 위치 저장
                found = strstr(found + 1, query); // 다음 위치 검색
            }

            if (last_valid_found) {
                search_row = i;
                search_col = last_valid_found - text[i];
                highlight_search();
                return true;
            }
        }
    }

    return false; 
}

void stop_search1() {
    is_searching = false;
    search_query[0] = '\0';
    up_msgbar("Exit2.");

    // 탐색 위치로 커서 이동
    if (search_row >= 0 && search_col >= 0) {
        cursor_row = search_row;
        cursor_col = search_col;
    }

    
    display();
    d_stbar();
    d_msgbar();
}

void stop_search2() {
    is_searching = false;
    search_query[0] = '\0';
    up_msgbar("Exit2.");

    
    display();
    d_stbar();
    d_msgbar();
}


int escesc() {
    int first = getchar(); 
    int second = 0;

    if (first == '[') { 
        second = getchar(); 
        switch (second) {
            case 'A': return 1000; // 위쪽 화살표
            case 'B': return 1001; // 아래쪽 화살표
            case 'C': return 1002; // 오른쪽 화살표
            case 'D': return 1003; // 왼쪽 화살표
            case 'H': return 1004; // Home 키
            case 'F': return 1005; // End 키
            case '5': getchar(); return 1006; // Page Up 
            case '6': getchar(); return 1007; // Page Down
        }
    }

    return -1; 
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        // ./viva 실행 시 새로운 파일을 생성
        filenamed("no Name");
    } else {
        printf("Usage: ./viva\n");
        return 1;
    }

    // 콘솔 모드 설정
    set_console_mode();

    s_msgbar();
    s_stbar(filename);
    display();  // 프로그램 시작 시 텍스트 화면 출력
    d_stbar();       
    d_msgbar();      

    while (1) {
        // 커서 위치 갱신
        printf("\033[%d;%dH", cursor_row + 1, cursor_col + 1);
        fflush(stdout);

        int ch = _getch(); // 키보드 입력 감지
       #if __linux__
if (ch == '\033') { // Linux/macOS에서 Escape 키 감지
    int special_key = escesc(); // Escape 시퀀스 처리
    if (special_key != -1) { // 유효한 특별 키가 감지된 경우
        if (is_searching) {
            // 검색 중 방향키 처리
            if (special_key == 1000) { // 위 화살표
                if (!search_text(search_query, false)) {
                    up_msgbar("no found.");
                }
            } else if (special_key == 1001) { // 아래 화살표
                if (!search_text(search_query, true)) {
                    up_msgbar("no found.");
                }
            }
             continue; 
        } else {
            // 일반 방향키 처리
            switch (special_key) {
                case 1000: up(); break;
                case 1001: down(); break;
                case 1002: right(); break;
                case 1003: left(); break;
                case 1004: home(); break;
                case 1005: end(); break;
                case 1006: page_up(); break;
                case 1007: page_down(); break;
                default: break; // 처리되지 않은 키
            }
        }
    }
}
#endif

         if (ch == 17) { // Ctrl+Q
            quit_program();
        } else if (ch == 19) { // Ctrl+S
            save_file2();
        } 
        
         else if (ch == 6) { // Ctrl+F
            start_search();
        }

    else if (is_searching) { // 탐색 중 처리
    if (ch == '\n' || ch == '\r') { // Enter
        stop_search1();
    } else if (ch == 27) { // Esc
        stop_search2();
    } 
    
    else if (ch == 0 || ch == 224) { // 화살표 키 입력
        int arrow = _getch();
        if (arrow == 80) { // 화살표 아래: 다음 결과
            if (!search_text(search_query, true)) {
                up_msgbar("no found.");
            }
        } else if (arrow == 72) { // 화살표 위: 이전 결과
            if (!search_text(search_query, false)) {
                up_msgbar("no found.");
            }
        }
    } else if (ch >= 32 && ch <= 126) { // 검색어 입력
        int len = strlen(search_query);
        if (len < sizeof(search_query) - 1) {
            search_query[len] = ch;
            search_query[len + 1] = '\0';

            // 검색 상태 초기화
            search_row = 0;  // 첫 번째 줄부터 검색
            search_col = -1; // 검색 시작 위치를 초기화

            // 메시지 바에 검색어 표시
            char msg[256];
            snprintf(msg, sizeof(msg), "Search: %s", search_query);
            up_msgbar(msg);

            // 검색 실행
            if (!search_text(search_query, true)) {
                snprintf(msg, sizeof(msg), "Search: %s (no found)", search_query);
                up_msgbar(msg);
            }
        }
    } else if (ch == 127 ||ch == 8) { // Backspace
        int len = strlen(search_query);
        if (len > 0) {
            search_query[len - 1] = '\0'; // 검색어에서 마지막 문자 삭제

            // 검색 상태 초기화
            search_row = 0;  // 첫 번째 줄부터 검색
            search_col = -1; // 검색 시작 위치를 초기화

            // 메시지 바에 검색어 표시
            char msg[256];
            snprintf(msg, sizeof(msg), "Search: %s", search_query);
            up_msgbar(msg);

            // 검색 실행
            if (!search_text(search_query, true)) {
                snprintf(msg, sizeof(msg), "Search: %s (no found)", search_query);
                up_msgbar(msg);
            }
        }
    }
}

       

         else if(ch == 127 || ch == 8) { // Backspace
            if (cursor_col > 0 || cursor_row > 0) {
                backspace();
                display();
            d_stbar();
            d_msgbar();
                is_modified = true;
            }
        } else if (ch == '\n' || ch == '\r') { // Enter
            // 현재 줄의 텍스트를 명령어로 판단
            if (strcmp(text[cursor_row], "./viva viva.c") == 0) {
                open_file("viva.c");
                text[cursor_row][0] = '\0'; // 명령 실행 후 현재 줄 초기화
            } else if (strcmp(text[cursor_row], "./viva") == 0) {
                filenamed("viva.c");
                text[cursor_row][0] = '\0'; // 명령 실행 후 현재 줄 초기화
            } else {
                a_lines();
                cursor_col = 0; // 커서를 줄의 처음으로 이동
                is_modified = true;
            }

            
            display();
            d_stbar();
            d_msgbar();
        } else if (ch == 0 || ch == 224) { // 화살표 키와 같은 특수 키
            int arrow = _getch();
            switch (arrow) {
                case 72: // 화살표 위
                    up();
                    break;
                case 80: // 화살표 아래
                    down();
                    break;
                case 75: // 화살표 왼쪽
                    left();
                    break;
                case 77: // 화살표 오른쪽
                    right();
                    break;
                case 71: // Home 키
                    home();
                    break;
                case 79: // End 키
                    end();
                    break;
                case 73: // Page Up 키
                    page_up();
                    break;
                case 81: // Page Down 키
                    page_down();
                    break;

                    
            }
            display();
            d_stbar();
            d_msgbar();
            printf("\033[%d;%dH", cursor_row + 1, cursor_col + 1);
            fflush(stdout);
        } else if (ch >= 32 && ch <= 126) { // 일반 문자 입력
            insert(ch);
            is_modified = true;

            
            display();
            d_stbar();
            d_msgbar();
        }
    }
    return 0;
}
