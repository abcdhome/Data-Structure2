#ifndef STBAR_H
#define STBAR_H
#include "text.h"
#include "terminal.h" // Ensure this is included

typedef struct {
    char filename[256];
    int total_lines;
    int cursor_row;
    int cursor_line;
    int cursor_col;
} StBar;

void s_stbar(const char *filename);
void d_stbar();
void co_li(const char *filename);
void up_cursor(int new_line, int new_col);
void to_li();

#endif // STBAR_H
