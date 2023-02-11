#include <stdio.h>
#include <stdlib.h>

typedef unsigned char byte;
typedef const char *text;

byte memory[100];
byte flags;

#define OF 0x02 // Overflow flag

int sc_regInit() {
    flags = 0;
    return 0;
}
int sc_regGet(int reg, int *value) {
    if (reg != OF) return 2;
    *value = (flags & reg) != 0;
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
    int value;
    if (!sc_regGet(OF, &value)) printf("flags: %u %u\n", flags, value);
    sc_regSet(OF, 0);
    if (!sc_regGet(OF, &value)) printf("flags: %u %u\n", flags, value);
    sc_regSet(OF, 1);
    if (!sc_regGet(OF, &value)) printf("flags: %u %u\n", flags, value);
    sc_regSet(OF, 1);
    if (!sc_regGet(OF, &value)) printf("flags: %u %u\n", flags, value);
    sc_regSet(OF, 0);
    if (!sc_regGet(OF, &value)) printf("flags: %u %u\n", flags, value);
    return 0;
}
