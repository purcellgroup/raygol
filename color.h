#ifndef COLOR_H_
#define COLOR_H_
#include <stdio.h>

#define COLOR_PRINT_256(FG, BG, STR, ...) \
  do {                                    \
    printf("\033[38;5;%dm", FG);          \
    printf("\033[48;5;%dm", BG);          \
    printf(STR, ##__VA_ARGS__);           \
    printf("\033[0m");                    \
  } while(0)                              \

#endif // COLOR_H_
