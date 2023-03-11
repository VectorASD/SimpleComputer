#include <lib.h>
#include <stdio.h>
#include <stdlib.h>

int
main (int argc, char **args)
{
  sc_regTest ();
  printf ("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  sc_memoryTest ();
  printf ("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  sc_commandTest ();
  return 0;
}
