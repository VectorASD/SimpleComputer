#include <myReadKey.h>
#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <unistd.h> // alarm

int ALU(int command, int operand) {
  return 0;
}

void StopCU() {
  alarm(0);
  rk_upd_mem();
}

int CU() {
  int value, command, operand;
  sc_memoryGet (instruction, &value);
  if (sc_commandDecode (value, &command, &operand)) {
    sc_regSet(TF, 1);
    return 1;
  }
  
  
  
  
  if (instruction == MEMORY_SIZE - 1) sc_regSet(TF, 1);
  else instruction += 1;
  mem_pos = instruction;
  return 0;
}

void
signalCallback (int signal)
{
  switch (signal)
    {
    case SIGALRM:
      // mt_gotoXY (1, 1);
      // my_printf ("ALARM\n");
      CU();
      int ignor;
      sc_regGet(TF, &ignor);
      if (ignor) alarm(0);
      rk_upd_mem();
      break;
    case SIGUSR1: // reset'илка
      // mt_gotoXY (1, 1);
      // my_printf ("SIGUSR1");
      alarm (0);
      rk_clear ();
      break;
    default:
      break;
    }
}

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
  signal (SIGALRM, signalCallback);
  signal (SIGUSR1, signalCallback);

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
