#pragma once

#include <myBigChars.h>
#include <termios.h>

typedef enum {K_F5, K_F6, K_UP, K_LEFT, K_RIGHT, K_DOWN, K_ENTER, K_ESC, K_L, K_S, K_R, K_T, K_I, K_OTHER} Keys;
typedef struct termios Term;


int rk_readkey(Keys *key);
int rk_mytermsave();
int rk_mytermrestore();
int rk_mytermregime(int regime, int vtime, int vmin, int echo, int sigint);

void rk_print();
void rk_test();
