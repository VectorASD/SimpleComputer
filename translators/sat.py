import os
import sys

def exit(*args, **kw_args):
  print(*args, **kw_args)
  sys.exit()

limit = 100 # самое противненькое число, ибо памяти реально совсем ничего.

instr_set = {
  "READ": 10,
  "WRITE": 11,

  "LOAD": 20,
  "STORE": 21,

  "ADD": 30,
  "SUB": 31,
  "DIVIDE": 32,
  "MUL": 33,
  "MOD": 34,

  "JUMP": 40,
  "JNEG": 41,
  "JZ": 42,
  "HALT": 43,

  "MOVA": 71,
  "MOVR": 72,
}

def translator(code):
  mem = [0x4000] * limit
  
  for line in code.split("\n"):
    line = line.split(";", 1)[0]
    arr = line.split()
    # try: arr = arr[:arr.index(";")]
    # except ValueError: pass ... Но можно ещё так: line = line.split(";", 1)[0]
    
    if not arr: continue
    
    if len(arr) != 3: exit('В строке "%s" ожидалось 3 слова' % line)
    a, b, c = arr
    try: addr = int(a)
    except ValueError: exit('В строке "%s" первое слово "%s" - не число' % (line, a))
    
    if addr not in range(limit): exit('Адрес "%s" за пределами памяти (допустимо 0..%s)' % (addr, limit - 1))
    
    if b == "=":
      err = 'После слова "=" ожидалось ровно 5 символов формата <"+"|"-"><%02d-команда от 00 до 79><%02x-величина от 00 до 7f>'

      orig_c = c
      if not c or c[0] not in "+-": c = "+" + c
      while len(c) < 5: c = c[0] + "0" + c[1:]
      if c != orig_c: print('Содержимое ячейки преобразовано: "%s" -> "%s"' % (orig_c, c))
      
      if len(c) != 5: exit(err)
      sign, code, value = c[0], c[1:3], c[3:5]
      if sign not in "-+": exit(err)
      try: code, value = int(code), int(value, 16)
      except ValueError: exit(err)
      if code not in range(80) or value not in range(128): exit(err)
      
      res = (0 if sign == "+" else 0x4000) | int(str(code), 16) << 7 | value
    else:
      b = b.upper()
      err = 'В строке "%s" после "%s" ожидалось число (допустимо 0..%s)' % (line, b, limit - 1)
      
      try: code = instr_set[b]
      except KeyError: exit('В строке "%s" кодовое слово "%s" нет в базе' % (line, b))

      if b == "HALT" and c not in ("0", "00"):
        print('После слова "HALT" проигнорировано слово "%s"' % c)
        c = "0"
      try: value = int(c)
      except ValueError: exit(err)
      if value not in range(limit): exit(err)
      res = int(str(code), 16) << 7 | value
    
    mem[addr] = res
  return mem

def print_mem(mem):
  arr = []
  for n, cell in enumerate(mem):
    sign = "-" if cell >> 14 else "+"
    code = cell >> 7 & 0x7f
    value = cell & 0x7f
    arr.append("%s%02x%02x" % (sign, code, value))
    if n % 10 == 9:
      print(" ".join(arr))
      arr = []

code = """
00 READ 09 ; (Ввод А)
01 READ 10 ; (Ввод В)
02 LOAD 09 ; (Загрузка А в аккумулятор)
03 SUB 10 ; (Отнять В)
04 JNEG 07 ; (Переход на 07, если отрицательное)
05 WRITE 09 ; (Вывод А)
06 HALT 00 ; (Останов)
07 WRITE 10 ; (Вывод В)
08 HALT 00 ; (Останов)
09 = +0000 ; (Переменная А)
10 = +797f ; (Переменная В)

; (Проверка исправителя формата)
20 = 0
21 = +
22 = -
23 HALT 283012303
"""

mem = translator(code)
print("~" * 60)
print("И того, получаем:")
print_mem(mem)

CurPath = os.path.dirname(__file__)
with open(os.path.join(CurPath, "translated.mem"), "wb") as file: file.write(b"".join(bytes((i & 255, i >> 8)) for i in mem))
