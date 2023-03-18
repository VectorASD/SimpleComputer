#include <myReadKey.h>
#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <unistd.h> // alarm

/* Правила возвратных значений ALU:
> 1: ошибка
== 1: ALU ничего не делает
< 1: CU проворачивает условный переход, предварительно убрав минус из
возвратного значения.
*/
int
ALU (int command, int operand)
{
  int value;
  switch (command)
    {
    case 0x30: // ADD
      if (sc_memoryGet (operand, &value))
        {
          sc_regSet (TF, 1);
          return 5;
        }
      accumulator += value;
      break;
    case 0x31: // SUB
      if (sc_memoryGet (operand, &value))
        {
          sc_regSet (TF, 1);
          return 6;
        }
      accumulator -= value;
      break;
    case 0x32: // DIVIDE
      if (sc_memoryGet (operand, &value))
        {
          sc_regSet (TF, 1);
          return 7;
        }
      if (value == 0)
        {
          sc_regSet (DF, 1); // Низя делить на н0лик ;'-}}}
          sc_regSet (TF, 1);
          return 8;
        }
      accumulator /= value;
      break;
    case 0x33: // MUL
      if (sc_memoryGet (operand, &value))
        {
          sc_regSet (TF, 1);
          return 9;
        }
      accumulator *= value;
      break;
    case 0x41: // JNEG
      if (accumulator < 0)
        return -operand; // CU пойдёт условный переход и уберёт минус
      break;
    case 0x42: // JZ
      if (accumulator == 0)
        return -operand; // CU пойдёт условный переход и уберёт минус
      break;
    }
  return 1;
}

void
StopCU ()
{
  alarm (0);
  rk_upd_mem ();
}

int
CU ()
{
  int value, command, operand, err;
  sc_memoryGet (instruction, &value);
  if (sc_commandDecode (value, &command, &operand))
    {
      sc_regSet (TF, 1);
      return 1;
    }

  int next_inst = instruction + 1;

  switch (command)
    {
    // 1x
    case 0x10: // READ
      break;
    case 0x11: // WRITE
      break;
    // 2x
    case 0x20: // LOAD
      if (sc_memoryGet (operand, &accumulator))
        {
          sc_regSet (TF, 1);
          return 2;
        }
      break;
    case 0x21: // STORE
      if (sc_memorySet (operand, accumulator))
        {
          sc_regSet (TF, 1);
          return 3;
        }
      break;
    // 3x
    case 0x30: // ADD
    case 0x31: // SUB
    case 0x32: // DIVIDE
    case 0x33: // MUL
      err = ALU (operand, command);
      if (err)
        return err;
      break;
    // 4x
    case 0x40: // JUMP
      next_inst = command;
      break;
    case 0x41: // JNEG
    case 0x42: // JZ
      err = ALU (operand, command);
      if (err > 1)
        return err;
      if (err <= 0)
        next_inst = -err;
      break;
    case 0x43: // HALT
      sc_regSet (TF, 1);
      break;
    // my
    case 0x65: // ADDC
      break;
    default:
      sc_memorySet (20, flags); // dbg ОТКУДА TF???
      sc_regSet (EF, 1);
      // sc_regSet (TF, 1); ЧЁ?!
      sc_memorySet (21, flags); // dbg
      sc_regGet (EF, &value);
      sc_memorySet (22, value); // dbg
      return 4;
    }

  if (instruction == MEMORY_SIZE - 1)
    sc_regSet (TF, 1);
  else if (next_inst < 0 || next_inst >= MEMORY_SIZE)
    sc_regSet (OF, 1);
  else
    instruction = next_inst;
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
      CU ();
      rk_upd_mem ();
      int ignor;
      sc_regGet (TF, &ignor);
      if (ignor)
        alarm (0);
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
