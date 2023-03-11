#include <myReadKey.h>
#include <stdio.h>  // stdin
#include <stdlib.h> // exit
#include <unistd.h> // read

byte
upper_comparer (char *buff, text right)
{
  for (int i = 0;; i++)
    {
      char c = buff[i], c2 = right[i];
      if (c >= 'a' && c <= 'z')
        c -= ' ';
      if (c != c2)
        return 0;
      if (c == 0)
        return 1; // по сути аналогично c == 0 && c2 == 0
    }
}

// typedef enum {K_F5, K_F6, K_UP, K_LEFT, K_RIGHT, K_DOWN, K_ENTER, K_ESC,
// K_L, K_S, K_R, K_T, K_I, K_OTHER} Keys;
#define key_num 13  // Без K_OTHER
text key_data[] = { // Порядок важен в соответствии с перечислением Keys
  "\e[15~",         // F5
  "\e[17~",         // F6
  "\e[A",           // стрелка вверх
  "\e[D",           // стрелка влево
  "\e[C",           // стрелка вправо
  "\e[B",           // стрелка вниз
  "\n",             // enter
  "\e", // escape... не ожидал, что '\e' без ничего - escape!!!
  "L",      "S", "R", "T", "I"
};

int
rk_readkey (Keys *key)
{
  fflush (stdout); // прочистка внутренностей stdout
  char buff[7]
      = "\0\0\0\0\0\0"; // 7 байт - всегда нулевой терминатор - предохранитель
  read (fileno (stdin), buff, 6);
  // for (int i = 0; i < 6; i++) my_printf("%u ", buff[i]);
  // my_printf(" | %s |\n", buff[0] == 27 ? buff + 1 : buff);

  // printf("|| %u\n", '\e'); Действительно 27 = '\e' ... Угадаль!

  for (int k = 0; k < key_num; k++)
    if (upper_comparer (buff, key_data[k]))
      {
        *key = (Keys)k;
        return 0;
      }

  *key = K_OTHER;
  return 0;
}

Term rk_save;

int
rk_mytermsave ()
{
  if (tcgetattr (fileno (stdin), &rk_save))
    return 1;
  return 0;
}

int
rk_mytermrestore ()
{
  tcsetattr (fileno (stdin), TCSAFLUSH, &rk_save);
  return 0;
}

// regime: 1 - канонический, 0 - неканонический
// echo: 1 - будем видеть свои введённые символы, 0 - не будем
// sigint: 1 - можно использовать ctrl+... alt+..., включая (intr), (quit), 0 -
// на прямую сыпятся ctrl и alt, как отдельные символы

int
rk_mytermregime (int regime, int vtime, int vmin, int echo, int sigint)
{
  if (regime != !!regime)
    return 1;
  if (vtime < 0 || vmin < 0 || (echo != !!echo) || (sigint != !!sigint))
    return 2;
  Term term;
  if (tcgetattr (fileno (stdin), &term))
    return 3;

  if (!!(term.c_lflag & ICANON) != regime)
    term.c_lflag ^= ICANON;
  if (!!(term.c_lflag & ISIG) != sigint)
    term.c_lflag ^= ISIG;
  if (!!(term.c_lflag & ECHO) != echo)
    term.c_lflag ^= ECHO;
  term.c_cc[VMIN] = vmin;
  term.c_cc[VTIME] = vtime;

  tcsetattr (fileno (stdin), TCSAFLUSH, &term);
  return 0;
}

void
rk_print ()
{
  my_printf ("Первоначальные настройки:\n");
  Term term;
  if (tcgetattr (fileno (stdin), &term))
    {
      my_printf ("  Нет доступа к настройкам :/\n");
      exit (1);
    }
  my_printf ("  ICANON: %u\n", !!(term.c_lflag & ICANON));
  my_printf ("  ISIG: %u\n", !!(term.c_lflag & ISIG));
  my_printf ("  ECHO: %u\n", !!(term.c_lflag & ECHO));
  my_printf ("  VMIN: %u\n", term.c_cc[VMIN]);
  my_printf ("  VTIME: %u\n", term.c_cc[VTIME]);
}

void
rk_test ()
{
  // rk_print();
  if (rk_mytermsave ())
    {
      my_printf ("Не удалось сохранить настройки терминала\n");
      exit (1);
    }
  // my_printf("Настройки успешно сохранены\n");
  mt_clrscr ();
  rk_mytermregime (0, 1, 1, 0, 0);
  bc_start ();

  mt_gotoXY (27, 7);
  my_printf ("Input\\Output:");
  mt_gotoXY (28, 7);

  for (int i = 0;; i++)
    {
      Keys res;
      rk_readkey (&res);
      mt_gotoXY (28, 7);
      if (res != K_OTHER)
        my_printf ("Key_num: %u   ", res);
      else
        my_printf ("Other_key   ");
      if (res == K_ESC)
        break;
    }

  rk_mytermrestore ();
  // my_printf("Настройки успешно восстановлены\n");
  mt_ll ();
}
