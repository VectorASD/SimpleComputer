#include <myBigChars.h>
#include <stdio.h>

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

void load_glyph(FT_Face face, uint code, uint pos) {
    if (FT_Load_Char(face, code, FT_LOAD_RENDER)) {
        printf("Не удаётся загрузить глиф 0x%x\n", code);
        if (code == '?') exit(9);
        return load_glyph(face, '?', pos);
    }
    FT_Bitmap bitmap = face->glyph->bitmap;
    int width = bitmap.width, rows = bitmap.rows;
    int shift = (8 - rows) / 2;
    int A = 0, B = 0;
    for (int y = -shift; y < 8 - shift; y++) {
        byte b = 0;
        for (int x = -1; x < 7; x++) {
            byte alpha = x < 0 || x >= width || y >= rows ? 0 : bitmap.buffer[x + y * width];
            byte bit = alpha > 35;
            if (bit) b |= 1 << (6 - x);
            //printf("%u ", bit);
        }
        int norm_y = y + shift;
        if (norm_y < 4)
            A |= b << (8 * (3 - norm_y));
        else
            B |= b << (8 * (7 - norm_y));
        //printf("\n");
    }
    //printf("%x %x\n", A, B);
    glyph_table[pos] = A;
    glyph_table[pos + 1] = B;
}

void bc_glyphs_loader() {
    FT_Library ft; // в freetype/freetype.h на 1072 строчке описана эта структура
    FT_Face face;

    if (FT_Init_FreeType(&ft)) {
        printf("Не удаётся инициализировать FreeType библиотеку\n");
        exit(7);
    }
    if (FT_New_Face(ft, "fonts/CodenameCoderFree4F-Bold.ttf", 0, &face)) {
        printf("Не удаётся загрузить шрифт\n");
        exit(8);
    }

    FT_Set_Pixel_Sizes(face, 0, 13); // 13 размер для ttf воспринимается, как 8 пикселей в высоту и 6 в ширину
    for (int i = 0; i < 10; i++) load_glyph(face, '0' + i, i * 2);
    load_glyph(face, '+', 20);
    load_glyph(face, '-', 22);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void bc_tprintbigchar(uint pos, int x, int y, Color a, Color b) {
    int c[] = {glyph_table[pos], glyph_table[pos + 1]};
    bc_printbigchar(c, x, y, a, b);
}

void sc_termTest() {
    bc_glyphs_loader();
    if (bc_bigcharwrite(0, glyph_table, 12)) exit(1);
    for (int i = 0; i < 64; i++) glyph_table[i] = i * 0x1792231;
    byte terminate = 0; // Если поставить не 0, то все глифы будут испорчены, т.е. действительно файл помнит символы как надо ;'-}
    if (!terminate) {
        int count = 0;
        if (bc_bigcharread(0, glyph_table, 1000, &count)) exit(2);
        if (count != 12) exit(3);
    }
    //sc_printTable();
    mt_clrscr();
    bc_box(5, 3, 8 * 12 + 2, 8 * 2 + 4);
    for (int i = 0; i < 12; i++) bc_tprintbigchar(i * 2, 6 + i * 8, 4, RED, SUN);
    for (int i = 0; i < 12; i++) bc_tprintbigchar(i * 2 + 1, 6 + i * 8, 14, RED, SUN); // rusty glyphs
    mt_ll();
}
