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
	for (int i = 1; i < w-1; i++) str[i] = 'q'; 
	str[w-1] = 'k';
	mt_gotoXY(y1++, x1);
	bc_printA(str);
	
	str[0] = 'x';
	for (int i = 1; i < w-1; i++) str[i] = ' '; 
	str[w-1] = 'x';
	for (int i = 2; i < h; i++) {
		mt_gotoXY(y1++, x1);
		bc_printA(str);
	}
	
	str[0] = 'm';
	for (int i = 1; i < w-1; i++) str[i] = 'q'; 
	str[w-1] = 'j';
	mt_gotoXY(y1, x1);
	bc_printA(str);
	
	return 0;
}

int bc_printbigchar(int c[2], int x, int y, Color a, Color b) {
	return 0;
}
// - выводит на экран "большой символ" размером восемь строк на
// восемь столбцов, левый верхний угол которого располагается в строке x и
// столбце y. Третий и четвёртый параметры определяют цвет и фон выводимых
// символов. "Символ" выводится исходя из значений массива целых чисел
// следующим образом. В первой строке выводится 8 младших бит первого
// числа, во второй следующие 8, в третьей и 4 следующие. В 5 строке выводятся
// 8 младших бит второго числа и т.д. При этом если значение бита = 0, то
// выводится символ "пробел", иначе - символ, закрашивающий знакоместо
// (ACS_CKBOARD);

int bc_setbigcharpos(int *big, int x, int y, int value) {
	return 0;
}
// - устанавливает значение знакоместа "большого символа" в строке x и столбце
// y в значение value;

int bc_getbigcharpos(int *big, int x, int y, int *value) {
	return 0;
}
// - возвращает значение позиции в "большом символе" в строке x и столбце y;

int bc_bigcharwrite(int fd, int *big, int count) {
	return 0;
}
// - записывает заданное число "больших символов" в файл. Формат записи определяется
// пользователем;

int bc_bigcharread(int fd, int *big, int need_count, int *count) {
	return 0;
}
// - считывает из файла заданное количество "больших символов".
// Третий параметр указывает адрес переменной, в которую помещается
// количество считанных символов или 0, в случае ошибки.

void sc_printTable() {
	for (int i = 32; i < 128; i++) {
		printf("%c -> \e(0%c\e(B", i, i);
		printf(i % 16 == 15 ? "\n" : "    ");
	}
}
void sc_termTest() {
	sc_printTable();
	mt_clrscr();
	bc_box(5, 3, 10, 10);
	//bc_printA("lqqqqqk\n");
	//bc_printA("x     x\n");
	//bc_printA("mqqqqqj\n");
}
