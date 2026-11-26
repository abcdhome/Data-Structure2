#ifndef TERMINAL_H
#define TERMINAL_H

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#endif

// 터미널 크기를 가져오는 함수의 프로토타입
void terminal1(int *rows, int *cols);

#endif // TERMINAL_H

