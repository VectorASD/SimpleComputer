#include <stdio.h>
#include <stdlib.h>

typedef unsigned char byte;
typedef unsigned short ushort;
typedef const char *text;

#define MEMORY_SIZE 100
#define OF 0x02 // Overflow flag

ushort memory[MEMORY_SIZE];
byte flags;

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
void sc_regTest() {
    sc_regInit();
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
}

int sc_memoryInit() {
    for (int i = 0; i < MEMORY_SIZE; i++) memory[i] = 0;
    return 0;
}
int sc_memorySet(int address, int value) {
    if (address < 0 || address >= MEMORY_SIZE) {
        sc_regSet(OF, 1);
        return 1;
    }
    memory[address] = (ushort) value;
    return 0;
}
int sc_memoryGet(int address, int *value) {
    if (address < 0 || address >= MEMORY_SIZE) {
        sc_regSet(OF, 1);
        return 1;
    }
    *value = memory[address];
    return 0;
}
int sc_memorySave(text path) {
    FILE *file;
    if ((file = fopen(path, "wb")) == NULL) return 1;
    if (fwrite(memory, sizeof(ushort), MEMORY_SIZE, file) != MEMORY_SIZE) return 2;
    if (fclose(file) == EOF) return 3;
    return 0;
}
int sc_memoryLoad(text path) {
    FILE *file;
    if ((file = fopen(path, "rb")) == NULL) return 1;
    if (fread(memory, sizeof(ushort), MEMORY_SIZE, file) != MEMORY_SIZE) return 2;
    if (fclose(file) == EOF) return 3;
    return 0;
}
void sc_memoryPrint() {
    int value;
    printf("mem:");
    for (int i = 0; i < MEMORY_SIZE; i++)
        if (!sc_memoryGet(i, &value)) printf(" %u", value);
    printf("\n");
}
void sc_memoryTest() {
    sc_regInit();
    sc_memoryInit();

    printf("out_addrs:");
    for (int i = 0; i < MEMORY_SIZE; i++)
        if (sc_memorySet(i ^ 57, i + 10)) printf(" %u", i ^ 57);
    printf("\n\n");

    sc_memoryPrint();
    if (sc_memorySave("memory.mem")) {
        printf("Ошибка сохранения в файл\n");
        return;
    } else
        printf("Файл сохранился удачно\n\n");

    sc_memoryInit();
    sc_memoryPrint();

    if (sc_memoryLoad("memory.mem")) {
        printf("Ошибка загрузки из файла\n");
        return;
    } else
        printf("Файл загрузился удачно\n\n");
    sc_memoryPrint();
}

int sc_commandEncode(int command, int operand, int *value) {
    if (command < 0 || command > 127) return 1;
    if (operand < 0 || operand > 127) return 2;
    *value = (command << 7) | operand;
    return 0;
}
int sc_commandDecode(int value, int *command, int *operand) {
    if ((value >> 14) != 0) return 1; // не команда
    *command = value >> 7;
    *operand = value & 127;
    return 0;
}
void sc_commandTest() {
    for (int i = 0; i < 16; i++) {
        int value = (i * 0x231245141) % 0x7fff, command, operand;
        int err = sc_commandDecode(value, &command, &operand);
        if (err) {
            printf("%2u: %5u %3u %3u (%u)\n", i, value, command, operand, err);
            continue;
        }
        int value2;
        err = sc_commandEncode(command, operand, &value2);
        printf("%2u: %5u %3u %3u (%u) -> %u\n", i, value, command, operand, err, value2);
    }
}

int main(int argc, char **args) {
    // sc_regTest();
    //sc_memoryTest();
    sc_commandTest();

    return 0;
}
