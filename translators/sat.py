import os
import sys
import traceback

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
      err = 'После слова "=" ожидалось ровно 5 символов формата <"+"|"-"><%02x-команда от 00 до 7f><%02x-величина от 00 до 7f>'

      orig_c = c
      if not c or c[0] not in "+-": c = "+" + c
      while len(c) < 5: c = c[0] + "0" + c[1:]
      if c != orig_c: print('Содержимое ячейки преобразовано: "%s" -> "%s"' % (orig_c, c))
      
      if len(c) != 5: exit(err)
      sign, code, value = c[0], c[1:3], c[3:5]
      if sign not in "-+": exit(err)
      try: code, value = int(code, 16), int(value, 16)
      except ValueError: exit(err)
      if code not in range(128) or value not in range(128): exit(err)
      
      res = (0 if sign == "+" else 0x4000) | code << 7 | value
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
  
  print("~" * 60)
  print("И того, получаем:")
  print_mem(mem)
  
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
""" # добавил этот же код ещё и в sample.sa

# CurPath = os.path.dirname(__file__)
# with open(os.path.join(CurPath, "translated.mem"), "wb") as file: file.write(b"".join(bytes((i & 255, i >> 8)) for i in mem))

def main():
  import optparse
  parser = optparse.OptionParser(usage="sat.py <file_input_path.py> <file_output_path.mem>")
  parser.add_option("-t", "--test", action="store_true")
  options, args = parser.parse_args(sys.argv) # добавляет опцию --help и --test
  args = args[1:]
  if len(args) != 2: parser.error("Ожидалось 2 строки после sat.py")
  src, dist = args
  
  if options.test:
    d_dir = os.path.dirname(src + os.path.sep)
    if d_dir and not os.path.exists(d_dir): parser.error("❌ Не обнаружена 1-ая дирректория .mem файлов: '%s/'" % d_dir)
    d_dir2 = os.path.dirname(dist + os.path.sep)
    if d_dir2 and not os.path.exists(d_dir2): parser.error("❌ Не обнаружена 2-ая дирректория .mem файлов: '%s/'" % d_dir2)
    if d_dir == d_dir2: parser.error("❌ Обе дирректории одинаковые :/")
    A = set(name for name in os.listdir(d_dir) if name.endswith(".mem"))
    B = set(name for name in os.listdir(d_dir2) if name.endswith(".mem"))
    for name in A & B:
      path_a = os.path.join(d_dir, name)
      path_b = os.path.join(d_dir2, name)
      with open(path_a, "rb") as file: data = file.read()
      with open(path_b, "rb") as file: data2 = file.read()
      if data == data2: print('✅ Файлы безошибочно равны! "%s" == "%s"' % (path_a, path_b))
      else: print('❌ Ошибка сопоставления :///// "%s" != "%s"' % (path_a, path_b)) # благо все 6 файлов отбарабанили галочку ;'-}
    return
  
  d_dir = os.path.dirname(dist)
  if not os.path.exists(src): parser.error("❌ Не найден файл-источник: '%s'" % src)
  if d_dir and not os.path.exists(d_dir): parser.error("❌ Не обнаружена дирректория файла-результата: '%s/'" % d_dir)
  
  try:
    with open(src) as file: code = file.read()
  except:
    print("❌ Ошибка открытия файла-источника:\n%s" % traceback.format_exc())
    return

  try: mem = translator(code)
  except:
    print("❌ Ошибка транслирования:\n%s" % traceback.format_exc())
    return
  
  try:
    with open(dist, "wb") as file: file.write(b"".join(bytes((i & 255, i >> 8)) for i in mem))
    print("✅ Файл '%s' успешно сохранён" % dist)
  except: print("❌ Ошибка сохранения файла-результата:\n%s" % traceback.format_exc())

# mem = translator(code)

if __name__ == "__main__": main()
