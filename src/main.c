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

// Спасибо https://unicode-table.com/ru/blocks/box-drawing/
#define BOX_LU "╔"
#define BOX_RU "╗"
#define BOX_RD "╝"
#define BOX_LD "╚"
#define BOX_V "║"
#define BOX_H "═"
int str_len(text str) {
    int pos = 0;
    while (str[pos]) pos++;
    return pos;
}
void mt_printBox(int X, int Y, int SX, int SY, text title) {
    if (SX < 1 || SY < 1 || mt_gotoXY(++Y, ++X)) return;
    int orig_Y = Y, len = str_len(title);
    printf(BOX_LU);
    for (int i = 0; i < SX; i++) printf(BOX_H);
    printf(BOX_RU);
    for (int i = 0; i < SY; i++) {
        if (!mt_gotoXY(++Y, X)) printf(BOX_V);
        if (!mt_gotoXY(Y, X + SX + 1)) printf(BOX_V);
    }
    mt_gotoXY(++Y, X);
    printf(BOX_LD);
    for (int i = 0; i < SX; i++) printf(BOX_H);
    printf(BOX_RD);
    if (len) {
        mt_gotoXY(orig_Y, X + (SX - len) / 2);
        printf(" %s ", title);
    }
    mt_gotoXY(orig_Y + 1, X + 1);
}

void mt_printMemory(int X, int Y, int current) {
    mt_printBox(X, Y, 59, (MEMORY_SIZE + 9) / 10, "Memory");
    X += 2;
    Y += 2;
    for (int mem = 0; mem < MEMORY_SIZE; mem++) {
        int memX = mem % 10, memY = mem / 10;
        if (memX == 0 && memY) mt_gotoXY(Y + memY, X);
        if (memX) printf(" ");
        int value;
        sc_memoryGet(mem, &value);
        if (mem == current) {
            mt_setfgcolor(SUN);
            mt_setbgcolor(BLUE);
        }
        printf("+%04X", value);
        if (mem == current) mt_clrclr();
    }
}
void mt_printFlags(int X, int Y) {
    mt_printBox(X, Y, 25, 1, "Flags");
    X += 2;
    Y += 2;
    char buff[12];
    byte flags[5];
    int pos = 0, value;

    for (int i = 0; i < 5; i++) sc_regSet(1 << i, 1); // костыль для проверки

    sc_regGet(DF, &value); // Для максимальной правдаподобности,
    flags[0] = value;      // хоть руки и чешутся влубить цикл, где: reg = 1 << i
    sc_regGet(OF, &value);
    flags[1] = value;
    sc_regGet(MF, &value);
    flags[2] = value;
    sc_regGet(TF, &value);
    flags[3] = value;
    sc_regGet(EF, &value);
    flags[4] = value;

    for (int i = 0; i < 5; i++)
        if (flags[i]) {
            if (pos) buff[pos++] = ' ';
            buff[pos++] = "DOMTE"[i];
        }
    buff[pos] = 0;

    mt_gotoXY(Y, X + (25 - pos) / 2);
    printf("%s", buff);
}
void mt_printKeys(int X, int Y) {
    mt_printBox(X, Y, 40, 8, "Keys");
    X += 2;
    Y += 2;
    text keys[] = {"l  - load", "s  - save", "r  - run", "s  - step", "r  - reset", "F5 - accumulator", "F6 - instructionCounter"};
    for (int i = 0; i < 7; i++) {
        mt_gotoXY(Y + i, X);
        printf("%s", keys[i]);
    }
}
void mt_printBigNumbers(int X, int Y) { // TODO
    mt_printBox(X, Y, 8 * 5 + 4, 8, "");
    X += 2;
    Y += 2;
    for (int num = 0; num < 5; num++) {
        for (int row = 0; row < 8; row++) {
            mt_gotoXY(Y + row, X + num * 9);
            printf("[][][][]");
        }
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
    for (int i = 0; i < rows; i++) printf("\n"); // Очищает экран без потери выведенного до этого текста
    mt_printBox(6, 3, 20, 5, "YeahBox");
    mt_printBox(29, 3, 20, 5, "YeahBox2");
    mt_printMemory(6, 10, 44);
    mt_printFlags(68, 19);
    mt_printBigNumbers(6, 22);
    mt_printKeys(53, 22);
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
