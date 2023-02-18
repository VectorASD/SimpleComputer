#pragma once

typedef unsigned char byte;
typedef unsigned short ushort;
typedef const char *text;

#define MEMORY_SIZE 100
#define DF 0x01 // Division error flag
#define OF 0x02 // Overflow flag
#define MF 0x04 // Memory overflow flag
#define TF 0x08 // Tackts ignore flag
#define EF 0x10 // Error command flag

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

/* visual tests */

void sc_regTest();
void sc_memoryPrint();
void sc_memoryTest();
void sc_commandTest();
