#pragma once

#include <myBigChars.h>

typedef enum {F5, F6} Keys;

int rk_readkey(Keys *keys);
int rk_mytermsave();
int rk_mytermrestore();
int rk_mytermregime(int regime, int vtime, int vmin, int echo, int sigint);
