#pragma once

typedef unsigned char byte;
typedef unsigned short ushort;
typedef const char *text;

#define MEMORY_SIZE 100
#define OF 0x02 // Overflow flag

extern ushort memory[MEMORY_SIZE];
extern byte flags;

int sc_regInit();
int sc_regGet(int reg, int *value);
int sc_regSet(int reg, int value);

int sc_memoryInit();
int sc_memorySet(int address, int value);
int sc_memoryGet(int address, int *value);

int sc_memorySave(text path);
int sc_memoryLoad(text path);

int sc_commandEncode(int command, int operand, int *value);
int sc_commandDecode(int value, int *command, int *operand);
