#include <lib.h>
#include <myTerm.h>
#include <stdio.h>
#include <stdlib.h>

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

void mt_termTest() {
    int rows, cols;
    mt_getscreensize(&rows, &cols);
    printf("screen sizes: %ux%u\n", cols, rows);
    printf("   Фон:     DEFAULT   Буквы:     DEFAULT\n");
    text Names[] = {"Black", "Red", "Green", "Brown", "Blue", "Magent", "Cyan", "DarkGray", "LightGray", "Pink", "Lime", "Sun", "Aqua", "LightMagent", "LightCyan", "White"};
    for (int i = 0; i < 16; i++) {
        printf("   ");
        mt_setfgcolor(i);
        mt_setbgcolor(15 - i);
        printf("Фон: %11s   Буквы: %11s", Names[i], Names[15 - i]);
        mt_clrclr();
        printf("\n");
    }
    mt_setfgcolor(SUN);
    mt_setbgcolor(BLUE);
    for (int i = 0; i < 16; i++)
        if (!mt_gotoXY((i ^ 5) + 1, i + 40)) printf("YEAH! %u", i);
    mt_clrclr();
    mt_ll();
}

int main(int argc, char **args) {
    sc_regTest();
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    sc_memoryTest();
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    sc_commandTest();
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    mt_termTest();
    return 0;
}
