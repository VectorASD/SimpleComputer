#include <myReadKey.h>
#include <stdio.h>
#include <stdlib.h>

int
main (int argc, char **args)
{
  int rows, cols;
  mt_getscreensize (&rows, &cols);
  if (cols < 101 || rows < 31)
    {
      my_printf ("Минимально допустимый размер консоли: (101;31)\n");
      my_printf ("Текущий размер консоли: (%u;%u)\n", cols, rows);
      exit (1);
    }
  if (rk_mytermsave ())
    {
      my_printf ("Не удалось сохранить настройки терминала\n");
      exit (2);
    }
  mt_clrscr ();
  rk_mytermregime (0, 1, 1, 0, 0);

  sc_regSet (TF, 1); // Только флаг игнора тактов
  bc_start ();

  mt_gotoXY (27, 7);
  my_printf ("Input\\Output:");
  mt_gotoXY (28, 7);

  for (int i = 0;; i++)
    {
      Keys res;
      rk_readkey (&res);
      rk_key_handler (res);
      if (res == K_ESC)
        break;
    }

  rk_mytermrestore ();
  mt_ll ();
  return 0;
}
