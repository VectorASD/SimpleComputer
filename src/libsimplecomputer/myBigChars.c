#include <myBigChars.h>
#include <stdio.h>
#include <stdlib.h> // exit

int
bc_printA (text str)
{
  my_printf ("\e(0%s\e(B", str);
  return 0;
}

int
bc_box (int x1, int y1, int w, int h)
{ // К сожалению автор ТЗ перепутал X и Y, а также перепутал x2, y2 с w, h, что
  // приводит к серьёзной путанице :///
  if (x1 < 0 || y1 < 0 || w < 1 || h < 1)
    return 1;
  x1 += 1;
  y1 += 1;
  w += 2;
  h += 2;
  char str[w + 1];
  str[w] = 0;

  str[0] = 'l';
  for (int i = 1; i < w - 1; i++)
    str[i] = 'q';
  str[w - 1] = 'k';
  mt_gotoXY (y1++, x1);
  bc_printA (str);

  str[0] = 'x';
  for (int i = 1; i < w - 1; i++)
    str[i] = ' ';
  str[w - 1] = 'x';
  for (int i = 2; i < h; i++)
    {
      mt_gotoXY (y1++, x1);
      bc_printA (str);
    }

  str[0] = 'm';
  for (int i = 1; i < w - 1; i++)
    str[i] = 'q';
  str[w - 1] = 'j';
  mt_gotoXY (y1, x1);
  bc_printA (str);

  return 0;
}

int
bc_printbigchar (int c[2], int x, int y, Color a, Color b)
{
  mt_setfgcolor (a);
  mt_setbgcolor (b);
  char str[9];
  str[8] = 0;
  int pix;

  for (int i = 0; i < 8; i++)
    {
      mt_gotoXY (y++, x);
      for (int j = 0; j < 8; j++)
        {
          bc_getbigcharpos (c, j, i, &pix);
          str[j] = pix ? 'a' : ' ';
        }
      bc_printA (str);
    }

  mt_clrclr ();
  return 0;
}

int
bc_setbigcharpos (int *big, int x, int y, int value)
{
  if (x < 0 || x > 7 || y < 0 || y > 7)
    return 1;
  if (value != 0 && value != 1)
    return 2;
  int *num = y < 4 ? big : big + 1;
  int bit = (3 - (y & 3)) << 3 | (7 - x);
  if (((*num) >> bit & 1) != value)
    (*num) ^= 1 << bit;
  return 0;
}
int
bc_getbigcharpos (int *big, int x, int y, int *value)
{
  *value = 0;
  if (x < 0 || x > 7 || y < 0 || y > 7)
    return 1;
  byte B = y < 4 ? big[0] >> ((3 - y) << 3) : big[1] >> ((7 - y) << 3);
  *value = B >> (7 - x) & 1;
  return 0;
}

int
bc_bigcharwrite (int fd, int *big, int count)
{ // Зачем fd ? FILE* без звёздочки будет не правильно передавать...
  FILE *file;
  if ((file = fopen ("glyph_base.asd", "wb")) == NULL)
    return 1;
  if (fwrite (big, sizeof (int) * 2, count, file) != count)
    return 2;
  if (fclose (file) == EOF)
    return 3;
  return 0;
}
int
bc_bigcharread (int fd, int *big, int need_count, int *count)
{
  FILE *file;
  *count = 0;
  if ((file = fopen ("glyph_base.asd", "rb")) == NULL)
    return 1;
  int finded = fread (big, sizeof (int) * 2, need_count, file);
  if (fclose (file) == EOF)
    return 3;
  *count = finded;
  return 0;
}

//
// Далее идут мои методы, т.е. те, что не описаны в ТЗ
//

void
bc_printTable ()
{
  for (int i = 32; i < 128; i++)
    {
      my_printf ("%c -> \e(0%c\e(B", i, i);
      my_printf (i % 16 == 15 ? "\n" : "    ");
    }
}

int glyph_table[64]; // приватная таблица символов

void
bc_tprintbigchar (uint pos, int x, int y, Color a, Color b)
{
  int c[] = { glyph_table[pos], glyph_table[pos + 1] };
  bc_printbigchar (c, x, y, a, b);
}

int
bc_printBox (int x1, int y1, int w, int h, text title)
{
  int res = bc_box (x1, y1, w, h);
  if (res)
    return res;

  int len = str_len (title);
  if (len)
    {
      mt_gotoXY (y1 + 1, x1 + (w - len) / 2 + 1);
      my_printf (" %s ", title);
    }
  return 0;
}

void
bc_printMemory (int X, int Y, int current)
{
  X += 2;
  Y += 2;
  for (int mem = 0; mem < MEMORY_SIZE; mem++)
    {
      int memX = mem % 10, memY = mem / 10;
      if (memX == 0)
        mt_gotoXY (Y + memY, X);
      if (memX)
        my_printf (" ");
      int value, command, operand;
      sc_memoryGet (mem, &value);
      if (mem == current)
        {
          mt_setfgcolor (SUN);
          mt_setbgcolor (BLUE);
        }
      sc_commandDecode (value, &command, &operand);
      my_printf ("%c%02X%02X", value >> 14 & 1 ? '-' : '+', command, operand);
      if (mem == current)
        mt_clrclr ();
    }
}
void
bc_printFlags (int X, int Y)
{
  X += 2;
  Y += 2;
  char buff[12];
  byte flags[5];
  int pos = 0, value;

  for (int i = 0; i < 5; i++)
    sc_regSet (1 << i, 1); // костыль для проверки

  sc_regGet (DF, &value); // Для максимальной правдаподобности,
  flags[0] = value; // хоть руки и чешутся влупить цикл, где: reg = 1 << i
  sc_regGet (OF, &value);
  flags[1] = value;
  sc_regGet (MF, &value);
  flags[2] = value;
  sc_regGet (TF, &value);
  flags[3] = value;
  sc_regGet (EF, &value);
  flags[4] = value;

  for (int i = 0; i < 5; i++)
    if (flags[i])
      {
        if (pos)
          buff[pos++] = ' ';
        buff[pos++] = "DOMTE"[i];
      }
  buff[pos] = 0;

  mt_gotoXY (Y, X + (25 - pos) / 2);
  my_printf ("%s", buff);
}
void
bc_printKeys (int X, int Y)
{
  X += 2;
  Y += 2;
  text keys[] = { "l  - load",
                  "s  - save",
                  "r  - run",
                  "t  - step",
                  "i  - reset",
                  "F5 - accumulator",
                  "F6 - instructionCounter" };
  for (int i = 0; i < 7; i++)
    {
      mt_gotoXY (Y + i, X);
      my_printf ("%s", keys[i]);
    }
}

/*
Расположение символов в таблице глифов:
 0.. 1   0   |  2.. 3   1   |  4.. 5   2   |  6.. 7   3   |  8.. 9   4
10..11   5   | 12..13   6   | 14..15   7   | 16..17   8   | 18..19   9
20..21   +   | 22..23   -   | 24..25   a   | 26..27   b   | 28..29   c
30..31   d   | 32..33   e   | 34..35   f   | 36..63   reserved
*/
void
bc_printBigNumbers (int X, int Y, int num, Color a, Color b)
{
  if (num >= 0 && num < MEMORY_SIZE)
    sc_memoryGet (num, &num);
  X += 2;
  Y += 2;
  bc_tprintbigchar (num >> 14 & 1 ? 22 : 20, X, Y, a, b);
  for (int n = 0; n < 4; n++)
    {
      int let = num >> ((3 - n) * 4) & 15;
      if (!n)
        let &= 3;
      if (let > 9)
        let += 2;
      bc_tprintbigchar (let * 2, X + (n + 1) * 9, Y, a, b);
    }
}

void
bc_printAllBoxes ()
{
  bc_printBox (6, 3, 59, (MEMORY_SIZE + 9) / 10, "Memory");
  bc_printBox (68, 3, 25, 1, "accumulator");
  bc_printBox (68, 6, 25, 1, "instructionCounter");
  bc_printBox (68, 9, 25, 1, "Operation");
  bc_printBox (68, 12, 25, 1, "Flags");
  bc_printBox (6, 15, 8 * 5 + 4, 8, "");
  bc_printBox (53, 15, 40, 8, "Keys");
}

void
bc_printAccumulator (int accumulator)
{
  mt_gotoXY (5, 80);
  my_printf ("%c%04x", accumulator >> 15 & 1 ? '-' : '+', accumulator);
}

void
bc_printInstrCounter (int instr)
{
  mt_gotoXY (8, 80);
  my_printf ("+%04x", instr);

  int value, command, operand;
  sc_memoryGet (instr, &value);
  sc_commandDecode (value, &command, &operand);
  mt_gotoXY (11, 79);
  my_printf ("+%02x : %02x", command, operand);
}

void
bc_start ()
{
  int count;
  if (bc_bigcharread (0, glyph_table, 1000, &count))
    {
      my_printf ("Не обнаружен glyph_base.asd файл!\n");
      exit (2);
    }
  if (count != 18)
    exit (3);

  bc_printAllBoxes ();

  int current = 44;
  int accumulator = 0x1234;

  bc_printMemory (6, 3, current);
  bc_printFlags (68, 12);
  bc_printKeys (53, 15);
  // bc_printBigNumbers (68, 0, 0b111010000101111, WHITE, BLUE);
  bc_printBigNumbers (6, 15, current, RED, SUN);
  bc_printAccumulator (accumulator);
  bc_printInstrCounter (0);
}

void
bc_termTest ()
{
  if (sc_memoryLoad ("memory.mem"))
    {
      my_printf ("Ошибка загрузки из файла\n");
      return;
    }
  else
    my_printf ("Файл загрузился удачно\n\n");

  // if (bc_bigcharwrite(0, glyph_table, 18)) exit(1);
  // for (int i = 0; i < 64; i++) glyph_table[i] = i * 0x1792231;

  // bc_printTable();

  mt_clrscr ();
  bc_start ();
  mt_ll ();
}
