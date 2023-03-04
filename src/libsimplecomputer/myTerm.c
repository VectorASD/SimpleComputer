#include <myTerm.h>
#include <stdio.h>
#include <sys/ioctl.h> // ioctl
#include <unistd.h>    // STDOUT_FILENO

int mt_clrscr() {
    printf("\e[H\e[J");
    return 0;
}

int mt_gotoXY(int Y, int X) {
    int rows, cols;
    mt_getscreensize(&rows, &cols);
    if (X < 1 || Y < 1 || X > cols || Y > rows) return -1;
    printf("\e[%d;%dH", Y, X);
    return 0;
}

int mt_getscreensize(int *rows, int *cols) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    *rows = w.ws_row;
    *cols = w.ws_col;
    return 0;
}

int mt_setfgcolor(Color color) {
    if (color < 0 || color > 15) return -1;
    if (color == 7)
        color = 8;
    else if (color == 8)
        color = 7; // Восстанавливает порядок в палитре, как в Паскале.
    printf("\e[%d%dm", color < 8 ? 3 : 9, color & 7);
    return 0;
}
int mt_setbgcolor(Color color) {
    if (color < 0 || color > 15) return -1;
    if (color == 7)
        color = 8;
    else if (color == 8)
        color = 7; // Восстанавливает порядок в палитре, как в Паскале.
    printf("\e[%d%dm", color < 8 ? 4 : 10, color & 7);
    return 0;
}
int mt_clrclr() { // clear color. В лабораторной забыли добавить эту штуку.
    printf("\e[0m");
    return 0;
}

int mt_ll() { // last line. А вот это уже отсебятина, как любила говорить наш учитель-литератор после очередной проверки ЭССЕ ;'-}
    int rows, cols;
    mt_getscreensize(&rows, &cols);
    printf("\e[%d;%dH", rows, 1);
    return 0;
}

/* visual tests */

void mt_termTest() {
    int rows, cols;
    mt_getscreensize(&rows, &cols);
    printf("screen sizes: %ux%u\n", cols, rows);
    printf("   Фон:     DEFAULT   Буквы:     DEFAULT\n");
    text Names[] = {"Black", "Red", "Green", "Brown", "Blue", "Magent", "Cyan", "DarkGray", "LightGray", "Pink", "Lime", "Sun", "Aqua", "LightMagent", "LightCyan", "White"};
    for (int i = 0; i < 16; i++) {
        printf("   ");
        mt_setfgcolor(15 - i);
        mt_setbgcolor(i);
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
    //for (int i = 0; i < rows; i++) printf("\n"); // Очищает экран без потери выведенного до этого текста
    //mt_printBox(6, 3, 20, 5, "YeahBox");
    //mt_printBox(29, 3, 20, 5, "YeahBox2");
    //mt_printMemory(6, 10, 44);
    //mt_printFlags(68, 19);
    //mt_printBigNumbers(6, 22);
    //mt_printKeys(53, 22);
    //mt_ll();
}
