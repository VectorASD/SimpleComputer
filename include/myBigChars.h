#pragma once

#include <myTerm.h>

int bc_printA(text str);
int bc_box(int x1, int y1, int w, int h);
int bc_printbigchar(int c[2], int x, int y, Color a, Color b);
int bc_setbigcharpos(int *big, int x, int y, int value);
int bc_getbigcharpos(int *big, int x, int y, int *value);
int bc_bigcharwrite(int fd, int *big, int count);
int bc_bigcharread(int fd, int *big, int need_count, int *count);

void bc_printTable();
void bc_tprintbigchar(uint pos, int x, int y, Color a, Color b);
int bc_printBox(int x1, int y1, int w, int h, text title);
void bc_printBigNumbers(int X, int Y, int num, Color a, Color b);
void bc_termTest();
