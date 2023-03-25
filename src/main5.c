#include <myReadKey.h>
#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <unistd.h> // alarm

/* Сводка по файлам памяти:
memory.mem - первый файлик памяти, что доказывает работоспособность команд:
        READ, WRITE, LOAD, STORE, ADD, SUB, DIVIDE, MUL, MOD, JUMP, HALT.
        Нет только JNEG и JZ, и своих/пользовательских MOVA и MOVR.
        MOD по сути тоже кастомный, но без него больно.

        Вводим числа 'a' и 'b', получаем a+b, a-b, a//b, a%b, a*b

custom.mem - как раз проверяет команды:
        JNEG, JZ, MOVA и MOVR в полевых условиях.
        Как раз тут показывается вся сила командной работы DIV и MOD. ;'-}

        Вводим число 'a' и 'b', получаем в BIG-ENDIAN порядке символы 'a' числа
в системе счисления 'b'. Умный вариант с BIG-ENDIAN в отличие от простого
варианта little-endian базируется на массиве длины 15. ;'-}

        Не хватает команд ADDC и SUBC, чтобы в памяти += 1 и -= 1
соответственно, но обойдусь.
*/

/* Правила возвратных значений ALU:
>= 20: ошибка
== 1: ALU ничего не делает
<= 0: CU проворачивает условный переход, предварительно убрав минус из
вернутого значения.
*/
int
ALU (int command, int operand)
{
  int value;
  switch (command)
    {
    case 0x30: // ADD
      if (sc_memoryGet (operand, &value))
        return 20;
      accumulator += value;
      break;
    case 0x31: // SUB
      if (sc_memoryGet (operand, &value))
        return 21;
      accumulator -= value;
      break;
    case 0x32: // DIVIDE
      if (sc_memoryGet (operand, &value))
        return 22;
      if (value == 0)
        {
          sc_regSet (DF, 1); // Низя делить на н0лик ;'-}}}
          return 23;
        }
      accumulator /= value;
      break;
    case 0x33: // MUL
      if (sc_memoryGet (operand, &value))
        return 24;
      accumulator *= value;
      break;
    case 0x34: // MOD (моя прееелееесть!)
      if (sc_memoryGet (operand, &value))
        return 25;
      accumulator %= value;
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
  accumulator &= 0x7fff;
  return 1;
}

int cons_pos = 0;

void
comm_rw_clear ()
{
  if (cons_pos == 0)
    return;
  cons_pos = 0;
  int rows, cols;
  mt_getscreensize (&rows, &cols);
  for (int row = 28; row <= rows; row++)
    {
      mt_gotoXY (row, 7);
      my_printf ("                                                ");
    }
}

int
comm_read (int mem_pos)
{
  while (1)
    {
      int rows, cols;
      mt_getscreensize (&rows, &cols);
      if (28 + cons_pos >= rows)
        {
          sc_regSet (OF, 1);
          return 1;
        }

      mt_gotoXY (28 + cons_pos, 7);
      my_printf ("%u< ", mem_pos);
      int vvod = rk_common_mode (0);

      if (vvod == -2)
        return 2;
      if (vvod == -1)
        {
          mt_gotoXY (28 + cons_pos, 7);
          my_printf ("%u< Ничего не введено", mem_pos);
          cons_pos += 1;
        }
      else if (vvod > 32767)
        {
          mt_gotoXY (28 + cons_pos, 7);
          my_printf ("%u< Можно вводить только от -16383 до 16383", mem_pos);
          cons_pos += 1;
        }
      else
        {
          cons_pos += 1;
          if (sc_memorySet (mem_pos, vvod))
            return 3;
          return 0;
        }
    }
}

int
comm_write (int mem_pos)
{
  int rows, cols;
  mt_getscreensize (&rows, &cols);
  if (28 + cons_pos >= rows)
    {
      sc_regSet (OF, 1);
      return 1;
    }

  int data;
  if (sc_memoryGet (mem_pos, &data))
    return 2;

  mt_gotoXY (28 + cons_pos, 7);
  my_printf ("%u> %u", mem_pos, data);
  cons_pos += 1;
  return 0;
}

int
CU ()
{
  int value, command, operand, err;
  sc_memoryGet (instruction, &value);
  if (sc_commandDecode (value, &command, &operand))
    return 1;

  int next_inst = instruction + 1;

  switch (command)
    {
    // 1x
    case 0x10: // READ
      if (comm_read (operand))
        return 4;
      break;
    case 0x11: // WRITE
      if (comm_write (operand))
        return 5;
      break;
    // 2x
    case 0x20: // LOAD
      if (sc_memoryGet (operand, &accumulator))
        return 6;
      break;
    case 0x21: // STORE
      if (sc_memorySet (operand, accumulator))
        return 7;
      break;
    // 3x
    case 0x30: // ADD
    case 0x31: // SUB
    case 0x32: // DIVIDE
    case 0x33: // MUL
    case 0x34: // MOD (моя прееелееесть!)
      err = ALU (command, operand);
      if (err > 1)
        return err;
      if (err <= 0)
        next_inst = -err;
      break;
    // 4x
    case 0x40: // JUMP
      next_inst = operand;
      break;
    case 0x41: // JNEG
    case 0x42: // JZ
      err = ALU (command, operand);
      if (err > 1)
        return err;
      if (err <= 0)
        next_inst = -err;
      break;
    case 0x43: // HALT
      sc_regSet (TF, 1);
      next_inst = instruction;
      break;
    // my (7x). Т.к. можно было выбрать любой, то я выбираю умышленно конкретно
    // те 2, что позволяют работать с массивами ;'-} Опыт позволяет быстро
    // определить, какая команда это мне и позволяет проворачивать ;'-} Он же
    // (опыт) подсказывает, что на деле очень не хватает LOADR и SAVER
    // (названия с неба взял),
    //     чтобы вместо адреса ячейки я указывал адрес ячейки адреса ячейки :/
    case 0x71: // MOVA
      if (sc_memoryGet (operand, &value))
        return 8;
      if (sc_memorySet (accumulator, value))
        return 9;
      break;
    case 0x72: // MOVR
      if (sc_memoryGet (accumulator, &value))
        return 10;
      if (sc_memorySet (operand, value))
        return 11;
      break;
    default:
      sc_regSet (EF, 1);
      return 2;
    }

  if (next_inst >= MEMORY_SIZE || next_inst < 0)
    {
      sc_regSet (MF, 1);
      return 3;
    }

  if (next_inst < 0 || next_inst >= MEMORY_SIZE)
    sc_regSet (OF, 1);
  else
    instruction = next_inst;
  mem_pos = instruction;
  return 0;
}

int
check_flags ()
{
  int res;
  sc_regGet (DF, &res); // Division error flag
  if (res)
    return 1;
  sc_regGet (OF, &res); // Overflow flag
  if (res)
    return 1;
  sc_regGet (MF, &res); // Memory overflow flag
  if (res)
    return 1;
  sc_regGet (EF, &res); // Error command flag
  if (res)
    return 1;
  return 0;
}

void
signalCallback (int signal)
{
  switch (signal)
    {
    case SIGALRM:
      if (check_flags ())
        {
          sc_regSet (TF, 1);
          alarm (0);
          break;
        }
      if (CU ())
        sc_regSet (TF, 1);
      // mt_gotoXY (1, 1);
      // my_printf ("flags: %u     ", flags);
      int ignor;
      sc_regGet (TF, &ignor);
      if (ignor || check_flags ())
        {
          sc_regSet (TF, 1);
          alarm (0);
        }
      rk_upd_mem ();
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
      my_printf ("Минимально допустимый размер консоли: 101 x 31\n");
      my_printf ("Текущий размер консоли: %u x %u\n", cols, rows);
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

      comm_rw_clear ();
      if (rk_key_handler (res))
        break;
    }

  rk_mytermrestore ();
  mt_ll ();
  return 0;
}
