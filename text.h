#ifndef TEXT_H
#define TEXT_H

#define MAX_LINES 1000
#define MAX_COLS  80

#include <stdbool.h>

extern char text[MAX_LINES][MAX_COLS]; // 텍스트 배열 외부 선언
extern int cursor_row;
extern int cursor_col;
extern char filename[256];
extern bool is_modified;
extern int input_row;

void home();
void end();
void page_up();
void page_down();
void up();
void down();
void left();
void right();
void display();
void insert(char ch);
void backspace();
void a_lines();



#endif // TEXT_H 