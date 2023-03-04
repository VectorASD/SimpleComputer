#pragma once

#include <lib.h>

typedef enum {BLACK, RED, GREEN, BROWN, BLUE, MAGENT, CYAN, DARK_GRAY, LIGHT_GRAY, PINK, LIME, SUN, AQUA, LIGHT_MAGENT, LIGHT_CYAN, WHITE} Color;

int mt_clrscr();
int mt_gotoXY(int Y, int X);
int mt_getscreensize(int *rows, int *cols);

int mt_setfgcolor(Color color);
int mt_setbgcolor(Color color);
int mt_clrclr();

int mt_ll();

/* visual tests */

void mt_printBox(int X, int Y, int SX, int SY, text title);
void mt_printBigNumbers(int X, int Y);
void mt_termTest();
