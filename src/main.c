#include <stdio.h>
#include <stdlib.h>

typedef unsigned char byte;
typedef const char *text;

byte memory[100];

int sc_memoryInit() {
	for (int i = 0; i < 100; i++) memory[i] = 0;
	return 0;
}

int main(int argc, char **args) {
	sc_memoryInit();
    printf("YEAH3!\n");
    return 0;
}
