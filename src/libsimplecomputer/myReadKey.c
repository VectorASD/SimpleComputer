#include <myReadKey.h>
#include <signal.h>
#include <stdio.h> // stdin, fgets (для ввода регистров и памяти)
#include <stdlib.h> // exit
#include <unistd.h> // read, raise
#include <sys/time.h>

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
  rk_save.c_lflag
      |= ECHO; // Предохранитель на тот случай, если приложение вылетит
  // во время разработок, то не надо будет тыкаться 3 часа носом невидимыми
  // символами в консоль ;'-}
  rk_save.c_lflag
      |= ISIG; // Оказывается отсутствие этой штуки попротивнее будет,
               // чем отсутствие ECHO, т.к. если пропинговать vk.com с
               // сохранившейся ошибкой, то выйти никак не получится + все
               // вкладки gedit отлетят ;'-}}}
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

/*
 * Основные мозги тут:
 */

int prev_mem_pos = 0, mem_pos = 0; // -1: аккумулятор; -2: счётчик команд
int accumulator = 0;
int instruction = 0;

void
rk_upd_mem ()
{
  if ((mem_pos >= 0 && mem_pos <= MEMORY_SIZE)
      || (prev_mem_pos >= 0 && prev_mem_pos <= MEMORY_SIZE))
    {
      bc_printMemory (mem_pos);
      if (mem_pos >= 0)
        bs_print_bigN (mem_pos);
      // if (mem_pos == instruction)
      //   bc_printInstrCounter (instruction, 0);
    }
  // if (mem_pos == -1 || prev_mem_pos == -1)
  bc_printAccumulator (accumulator, mem_pos == -1);
  // if (mem_pos == -2 || prev_mem_pos == -2)
  bc_printInstrCounter (instruction, mem_pos == -2);
  bc_printFlags ();

  prev_mem_pos = mem_pos;
  mt_gotoXY (28, 7);
}

void
rk_clear ()
{
  accumulator = 0;
  instruction = 0;
  sc_regInit ();
  sc_regSet (TF, 1);
  mem_pos = 0;
  rk_upd_mem ();
}

void
rk_clear_vvod ()
{
  mt_gotoXY (28, 7);
  my_printf (
      "                                                                ");
  mt_gotoXY (28, 7);
}

void
rk_common_mode ()
{
  rk_clear_vvod ();
  my_printf ("Ввод: ");
  rk_mytermrestore ();

  char str[256];
  // scanf("%s", str);
  fgets (str, 256, stdin);
  // my_printf("\n%u %u %u %u %u %u", str[0], str[1], str[2], str[3], str[4],
  // str[5]);

  int pos = 0, num = 0, let = 0, minus = 0, plus = 0;
  char c;
  while ((c = str[pos++]))
    {
      if (c >= 'A' && c <= 'Z')
        c += ' '; // lower case

      if (c >= '0' && c <= '9')
        {
          if (c == '0' && let == 0)
            continue;
          num = num << 4 | (c - '0');
          let++;
        }
      else if (c >= 'a' && c <= 'f')
        {
          num = num << 4 | (c - 'a' + 10);
          let++;
        }
      else if (c == '-')
        {
          minus = 0x4000;
          plus = 0;
        }
      else if (c == '+')
        {
          minus = 0;
          plus = 1;
        }
    }

  if (let || minus || plus)
    {
      sc_commandEncode (num >> 8 & 127, num & 127, &num);
      num |= minus;

      if (mem_pos >= 0 && mem_pos <= MEMORY_SIZE)
        {
          sc_memorySet (mem_pos, num);
        }
      else if (mem_pos == -1)
        accumulator = num;
      else if (mem_pos == -2)
        instruction = num >= MEMORY_SIZE ? MEMORY_SIZE - 1 : num < 0 ? 0 : num;

      rk_upd_mem ();
    }
  rk_mytermregime (0, 1, 1, 0, 0);
}

void
rk_key_handler (Keys key)
{
  rk_clear_vvod ();
  if (key == K_L)
    {
      my_printf ("Загрузка...");
      if (sc_memoryLoad ("memory.mem"))
        {
          mt_gotoXY (28, 7);
          my_printf ("Ошибка загрузки файла memory.mem");
          return;
        }
      rk_upd_mem ();
      mt_gotoXY (28, 7);
      my_printf ("Файл memory.mem загружен удачно");
    }
  else if (key == K_S)
    {
      my_printf ("Сохранение...");
      mt_gotoXY (28, 7);
      if (sc_memorySave ("memory.mem"))
        {
          my_printf ("Ошибка сохранения файла memory.mem");
          return;
        }
      my_printf ("Файл memory.mem сохранён удачно");
    }
  else if (key == K_R) {
    sc_regSet (TF, 0);
    struct itimerval nval, oval;
    nval.it_interval.tv_sec = 0;
    nval.it_interval.tv_usec = 100000;
    nval.it_value.tv_sec = 0;
    nval.it_value.tv_usec = 1;
    setitimer (ITIMER_REAL, &nval, &oval);
  } else if (key == K_T)
    raise (SIGALRM);
  else if (key == K_I)
    raise (SIGUSR1);
  else if (key == K_UP)
    {
      mem_pos = mem_pos < 0 ? (mem_pos == -1 ? -2 : -1)
                            : (mem_pos - 10 + MEMORY_SIZE) % MEMORY_SIZE;
      rk_upd_mem ();
      mt_gotoXY (28, 7);
      my_printf ("Up");
    }
  else if (key == K_LEFT)
    {
      mem_pos = mem_pos < 0 ? (mem_pos == -1 ? 9 : 39)
                            : (mem_pos - 1 + MEMORY_SIZE) % MEMORY_SIZE;
      rk_upd_mem ();
      mt_gotoXY (28, 7);
      my_printf ("Left");
    }
  else if (key == K_RIGHT)
    {
      mem_pos = mem_pos < 0 ? (mem_pos == -1 ? 10 : 40)
                            : (mem_pos + 1) % MEMORY_SIZE;
      rk_upd_mem ();
      mt_gotoXY (28, 7);
      my_printf ("Right");
    }
  else if (key == K_DOWN)
    {
      mem_pos = mem_pos < 0 ? (mem_pos == -1 ? -2 : -1)
                            : (mem_pos + 10) % MEMORY_SIZE;
      rk_upd_mem ();
      mt_gotoXY (28, 7);
      my_printf ("Down");
    }
  else if (key == K_F5)
    {
      mem_pos = -1;
      rk_upd_mem ();
      mt_gotoXY (28, 7);
      my_printf ("F5");
    }
  else if (key == K_F6)
    {
      mem_pos = -2;
      rk_upd_mem ();
      mt_gotoXY (28, 7);
      my_printf ("F6");
    }
  else if (key == K_ENTER)
    {
      rk_common_mode ();
      rk_clear_vvod ();
      printf ("OK");
    }
  else if (key == K_ESC)
    my_printf ("Goodbye");
  else if (key != K_OTHER)
    my_printf ("Key_num: %u", key);
  else
    my_printf ("Other_key");
}

void
rk_test () // Только для lab04. Перенесено в main5 для дальнейшего
           // редактирования!
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
      rk_key_handler (res);
      if (res == K_ESC)
        break;
    }

  rk_mytermrestore ();
  // my_printf("Настройки успешно восстановлены\n");
  mt_ll ();
}
