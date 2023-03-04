#include <myBigChars.h>
#include <stdio.h>
#include <stdlib.h> // exit

int bc_printA(text str) {
    printf("\e(0%s\e(B", str);
    return 0;
}

int bc_box(int x1, int y1, int w, int h) { // К сожалению автор ТЗ перепутал X и Y, а также перепутал x2, y2 с w, h, что приводит к серьёзной путанице :///
    char str[w + 1];
    str[w] = 0;

    str[0] = 'l';
    for (int i = 1; i < w - 1; i++) str[i] = 'q';
    str[w - 1] = 'k';
    mt_gotoXY(y1++, x1);
    bc_printA(str);

    str[0] = 'x';
    for (int i = 1; i < w - 1; i++) str[i] = ' ';
    str[w - 1] = 'x';
    for (int i = 2; i < h; i++) {
        mt_gotoXY(y1++, x1);
        bc_printA(str);
    }

    str[0] = 'm';
    for (int i = 1; i < w - 1; i++) str[i] = 'q';
    str[w - 1] = 'j';
    mt_gotoXY(y1, x1);
    bc_printA(str);

    return 0;
}

int bc_printbigchar(int c[2], int x, int y, Color a, Color b) {
    mt_setfgcolor(a);
    mt_setbgcolor(b);
    char str[9];
    str[8] = 0;

    for (int i = 0; i < 8; i++) {
        mt_gotoXY(y++, x);
        byte B = i < 4 ? c[0] >> (8 * (3 - i)) : c[1] >> (8 * (7 - i));
        for (int j = 0; j < 8; j++) str[j] = B >> (7 - j) & 1 ? 'a' : ' ';
        bc_printA(str);
    }

    mt_clrclr();
    return 0;
}

// Вообще не понял смысловую нагрузку этих двух функция и зачем вообще тут *big + звёздочки не там стояли
int bc_setbigcharpos(int *big, int x, int y, int *value) {
    *value = x << 16 | y;
    return 0;
}
int bc_getbigcharpos(int *big, int *x, int *y, int value) {
    *x = value >> 16;
    *y = value & 0xffff;
    return 0;
}

int bc_bigcharwrite(int fd, int *big, int count) { // Зачем fd ? FILE* без звёздочки будет не правильно передавать...
    FILE *file;
    if ((file = fopen("glyph_base.asd", "wb")) == NULL) return 1;
    if (fwrite(big, sizeof(int) * 2, count, file) != count) return 2;
    if (fclose(file) == EOF) return 3;
    return 0;
}
int bc_bigcharread(int fd, int *big, int need_count, int *count) {
    FILE *file;
    *count = 0;
    if ((file = fopen("glyph_base.asd", "rb")) == NULL) return 1;
    int finded = fread(big, sizeof(int) * 2, need_count, file);
    if (fclose(file) == EOF) return 3;
    *count = finded;
    return 0;
}

//
// Далее идут мои методы, т.е. те, что не описаны в ТЗ
//

void sc_printTable() {
    for (int i = 32; i < 128; i++) {
        printf("%c -> \e(0%c\e(B", i, i);
        printf(i % 16 == 15 ? "\n" : "    ");
    }
}

int glyph_table[64]; // приватная таблица символов

void bc_tprintbigchar(uint pos, int x, int y, Color a, Color b) {
    int c[] = {glyph_table[pos], glyph_table[pos + 1]};
    bc_printbigchar(c, x, y, a, b);
}

void sc_termTest() {
    int count = 0;
    if (bc_bigcharread(0, glyph_table, 1000, &count)) exit(2);
    if (count != 18) exit(3);
    
    //if (bc_bigcharwrite(0, glyph_table, 18)) exit(1);
    //for (int i = 0; i < 64; i++) glyph_table[i] = i * 0x1792231;
    
    //sc_printTable();
    mt_clrscr();
    bc_box(5, 3, 8 * 18 + 2, 8 * 2 + 4);
    for (int i = 0; i < 18; i++) bc_tprintbigchar(i * 2, 6 + i * 8, 4, RED, SUN);
    for (int i = 0; i < 18; i++) bc_tprintbigchar(i * 2 + 1, 6 + i * 8, 14, RED, SUN); // rusty glyphs
    mt_ll();
}
