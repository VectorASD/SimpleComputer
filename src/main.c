#include <stdio.h>
#include <stdlib.h>

typedef unsigned char byte;
typedef const char *text;

byte memory[100];
byte flags;

#define OF 0x01 // Overflow flag

int sc_regInit() {
    flags = 0;
    return 0;
}
int sc_regSet(int reg, int value) {
    if (value != 0 && value != 1) return 1;
    if (reg != OF) return 2;
    if (((flags & reg) != 0) != value) flags ^= reg;
    return 0;
}

int sc_memoryInit() {
    for (int i = 0; i < 100; i++) memory[i] = 0;
    return 0;
}

int main(int argc, char **args) {
    sc_regInit();
    sc_memoryInit();
    printf("flags: %u\n", flags);
    sc_regSet(OF, 0);
    printf("flags: %u\n", flags);
    sc_regSet(OF, 1);
    printf("flags: %u\n", flags);
    sc_regSet(OF, 1);
    printf("flags: %u\n", flags);
    sc_regSet(OF, 0);
    printf("flags: %u\n", flags);
    return 0;
}
