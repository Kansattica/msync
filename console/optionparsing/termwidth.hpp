#ifndef _TERMWIDTH_HPP_
#define _TERMWIDTH_HPP_

#ifdef _WIN32
#include <windows.h>

int term_width()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;

}

#elif defined(__linux__)
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>

int term_width()
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}
#elif
int term_width()
{
    return 80;
}
#endif

#endif