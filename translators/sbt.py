import os
import sys
import traceback
from Python2bin import linker, print_mem

err_prefix = None
def exit(*args, **kw_args):
  if err_prefix is not None: print(err_prefix, end = "")
  print(*args, **kw_args)
  sys.exit()

# От одного лишь вида условий реализации данного транслятора, у него шарики за ролики очень глубоко заезжают ;'-}
# К сожалению реализовать работу массивов или хотябы как-то выразить этот код в виде питона не получиться :/ По этому без MOVA и MOVR
# По хорошему этот язык ещё ниже уровнем, чем simple assembler из-за своих ограничений...
# smali и то является низкоуровневым, хоть и имет на счету около 200 видов операций, опять же из-за меток,only goto,линейности кода в целом...
# Думаю долго это делать не буду ;'-}

# P.S. в разработке понял в чём подвох... ещё предполагал, что там придётся постараться, так на практике и вышло...
# из-за регулярных выражений я серьёзные объёмы кода вытряхнул из своего Python2bin'а ;'-} смотрите функцию expr, там и ВЕЛОСИПЕДНЫЙ парсер я подогнал из головы ;D

AB = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
AB += AB.lower() + "_"
augassign = ['+=', '-=', '/=', '*=', '%=']

def translator(code):
  global err_prefix

  codes = [] # упрощённый накопитель команд
  def add(code, value = 0): codes.append([code, value])
  
  consts = [] # рассчитано только под числа и переменные
  def new_const(data = None):
    if data is not None:
      try: return "c%s" % consts.index(data)
      except ValueError: pass
    n = len(consts)
    consts.append(data)
    return "c%s" % n
  def is_const(reg):
    if reg[0] != "c": return False
    return True #consts[int(reg[1:])] is not None
  regs = [] # регистры нужны для промежуточных операций
  def new_reg():
    for n, i in enumerate(regs):
      if not i:
        regs[n] = True
        return "r%s" % n
    regs.append(True)
    return "r%s" % (len(regs) - 1)
  def free_reg(n):
    if n == -1: return
    if n[0] == "r":
      n = int(n[1:])
      if n in range(len(regs)): regs[n] = False
  vars = {} # накопитель переменных
  def new_var(name):
    try: return vars[name]
    except KeyError: var = vars[name] = new_const()
    return var
  labels = {} # накопитель меток
  def get_label(name):
    count = labels.get(name, 0)
    labels[name] = count + 1
    return ":" + name + "_" + hex(count)[2:]

  def expr(data):
    R = 0
    arr = []
    for let in data:
      sign = let in "+-"
      term = let in "*/%"
      eq = let in "=<>!"
      num = let.isdigit()
      islet = let in AB
      space = let in " \t"
      
      if R == 0: # режим простоя
        if sign: R = 1
        elif term or eq: exit("символ '%s' в странном месте" % let)
        elif num: nn, R = int(let), 2
        elif islet:
          arr.append(("var", let))
          R = 3
        elif space: pass
      elif R == 1: # режим встречи +- после простоя
        if sign or term or eq: exit("после '%s' не может быть '%s'" % (prev, let))
        elif num:
          nn = (-1 if prev == "-" else 1) * int(let)
          R = 2
        elif islet:
          arr.append(("0-" if prev == "-" else "var", let))
          R = 3
        elif space: exit("после унарного '%s' не может быть пробела" % prev)
      elif R == 2: # режим "после цифры"
        if sign or term or eq:
          arr.append(("num", nn))
          R = 4
        elif num: nn = nn * 10 + int(let)
        elif islet: exit("после цифры '%s' не может быть буквы '%s'" % (prev, let))
        elif space:
          arr.append(("num", nn))
          R = 5
      elif R == 3: # режим "после буквы"
        if sign or term or eq: R = 4
        elif num or islet: exit("после буквы '%s' не может быть буквы|цифры '%s'" % (prev, let))
        elif space: pass
      elif R == 4: # режим встречи +-*/%=<>! после буквы или цифры
        if arr[-1][0] != "op": arr.append(("op", prev))
        if sign or term:
          if prev in "=<>!" and sign: R = 1
          else: exit("после '%s' не может быть '%s'" % (prev, let))
        elif eq:
          new = arr[-1][1] + let
          if new not in ("==", "!=", "<>", "<=", ">=", "+=", "-=", "*=", "/=", "%="): exit("не бывает комбинационного символа '%s'" % new)
          arr[-1] = ("op", new)
        elif num: nn, R = int(let), 2
        elif islet:
          arr.append(("var", let))
          R = 3
        elif space: R = 0
      elif R == 5: # режим "после цифры"
        if sign or term or eq: R = 4
        elif num or islet: exit("после цифры '%s' не может быть буквы|цифры '%s'" % (prev, let))
        elif space: pass
      prev = let

    # режим конца строки
    if R in (0, 3, 5): pass
    elif R in (1, 4): exit("после символа '%s' не может быть обрыва" % prev)
    elif R == 2: arr.append(("num", nn))

    # обработка того, что вышло. Гарантируется, что начинается с буквы|числа, заканчива буквой|цифрой, а также есть чередование этого со знаками операций
    if len(arr) % 2 != 1: exit("почему-то массив %s чётной длины :/" % arr)
    for i, el in enumerate(arr):
      if i % 2:
        if el[0] != "op": exit("НЕчётный элемент массива %s должен быть op" % arr)
        arr[i] = el[1]
      elif el[0] == "op": exit("чётный элемент массива %s НЕ должен быть op" % arr)

    def walker(ops, reverse = False): # Вот вам и "обратная Польская запись"))) На деле, я как-то этот код изобрёл до того, как узнал о ней
      nonlocal arr
      if reverse: arr = arr[::-1]
      pos = 1
      while pos < len(arr):
        a, op, b = arr[pos-1 : pos+2]
        if op in ops: arr[pos-1 : pos+2] = [(b, op, a) if reverse else (a, op, b)]
        else: pos += 2
      if reverse: arr = arr[::-1]
    walker("*/%")
    walker("+-")
    walker(("==", "!=", "<>", ">=", "<=", ">", "<"))
    walker(("=", "+=", "-=", "*=", "/=", "%="), True)

    def handler(arr):
      L = len(arr)
      if L == 1: return handler(arr[0])
      if L == 2:
        Type, value = arr
        if Type == "num": reg = new_const(value)
        elif Type == "var": reg = new_var(value)
        elif Type == "0-":
          var = new_const(0)
          add(20, var) # LOAD <c<0>>
          reg = new_reg()
          add(31, new_var(value)) # SUB <v<value>>
          add(21, reg) # STORE <reg>
        else: exit("не известный тип элемента: %s" % (arr,))
        print(reg, "<-", "-%s" % value if Type == "0-" else value)
        return reg
      a, op, b = arr
      if op in ("=", "+=", "-=", "*=", "/=", "%="):
        if len(a) != 2 or a[0] != "var": exit("нельзя использовать операцию '%s' не для переменной, т.е. (%s) %s ..." % (op, a, op))
        dst = new_var(a[1])
        reg = handler(b)
        if op == "=":
          add(20, reg) # LOAD <reg>
          add(21, dst) # STORE <dst>
        else:
          add(20, dst) # LOAD <dst>
          add(augassign.index(op) + 30, reg) # ADD/SUB/DIVIDE/MUL/MOD <reg>
          add(21, dst) # STORE <dst>
        print(dst, op, reg)
        free_reg(reg)
        reg = dst
      elif op in "+-*/%":
        reg, reg2 = handler(a), handler(b)
        add(20, reg) # LOAD <reg>
        add(augassign.index(op + "=") + 30, reg2) # ADD/SUB/DIVIDE/MUL/MOD <reg2>
        orig_r = reg
        if is_const(reg): reg = new_reg()
        add(21, reg) # STORE <reg>
        print(reg, "=", orig_r, op, reg2)
        free_reg(reg2)
      elif op in ("==", "!=", "<>", ">=", "<=", ">", "<"): # подробнее о том, как я это рассчитывал смотрите в Python2bin.py скрипте примерно на 350 строчке
        if op == "<>": op = "!="
        reg, reg2 = handler(a), handler(b)
        add(20, reg) # LOAD <reg>
        label = get_label("cmp") # comparison
        label2 = get_label("cmpd") # comparison dropper
        if reg2[0] == "c" and consts[int(reg2[1:])] == 0: pass # зачем из числа вычитать 0 ?)
        else: add(31, reg2) # SUB <reg2>
        free_reg(reg2)
        if op in ("==", "!="):
          add(42, label) # JZ <label>
          add(20, new_const(int(op == "!="))) # LOAD (0 при '==', либо 1 при '!=')
          add(40, label2) # JUMP <label2>
          add(-1, label)
          add(20, new_const(int(op == "=="))) # LOAD (1 при '==', либо 0 при '!=')
        elif op in ("<", ">="):
          add(41, label) # JNEG <label>
          add(20, new_const(int(op == ">="))) # LOAD (0 при '<', либо 1 при '>=')
          add(40, label2) # JUMP <label2>
          add(-1, label)
          add(20, new_const(int(op == "<"))) # LOAD (1 при '<', либо 0 при '>=')
        elif op in (">", "<="):
          add(41, label) # JNEG <label>
          add(42, label) # JZ <label>
          add(20, new_const(int(op == ">"))) # LOAD (1 при '>', либо 0 при '<=')
          add(40, label2) # JUMP <label2>
          add(-1, label)
          add(20, new_const(int(op == "<="))) # LOAD (0 при '>', либо 1 при '<=')
        else: error("expr:comparison: пока не поддерживается '%s' операция" % value)
        add(-1, label2)
        orig_r = reg
        if is_const(reg): reg = new_reg()
        add(21, reg) # STORE <reg>
        print(reg, "=", orig_r, op, reg2)
      return reg
    
    reg = handler(arr)
    free_reg(reg)
    return reg
  
  prev_label = -1
  for line in code.split("\n"):
    arr = line.split()
    if not arr: continue
    
    err_prefix = 'В строке "%s" ' % line
    if len(arr) == 1: exit("минимальный размер массива: 2")
    
    try: label = int(arr[0])
    except ValueError: exit("1-ое слово - не число")
    if label <= prev_label: exit('1-ое слово <= "%s" из предыдущей строки, спасибо условиям ТЗ' % prev_label)
    prev_label = label
    
    code = arr[1]
    if code != "REM":
      label = ":line_%s" % label
      labels[label] = 1
      add(-1, label)

    def handler(arr): # для поддержки того, что после IF-выражения
      global err_prefix
      err_prefix = 'В строке "%s" ' % " ".join(arr)
      
      code = arr[1]
      if not code.isupper(): exit("2-ое слово имеет прописные буквы, спасибо условиям ТЗ")
      
      if code == "REM": pass # комментарий
      elif code in ("INPUT", "READ"):
        if len(arr) != 3: exit("после %s должно быть ровно 1 слово" % code)
        var = arr[2]
        if var not in AB: exit("3-ее слово должно быть одной буквой, спасибо условиям ТЗ") # в ТЗ забыли уточнить, что переменные должны быть прописные ;'-}}}
        var = new_var(var)
        add(10, var) # READ <var>
      elif code in ("OUTPUT", "WRITE", "PRINT"):
        if len(arr) != 3: exit("после %s должно быть ровно 1 слово" % code)
        var = arr[2]
        if var not in AB: exit("3-ее слово должно быть одной буквой, спасибо условиям ТЗ")
        var = new_var(var)
        add(11, var) # WRITE <var>
      elif code == "END":
        if not codes or codes[0][0] != 43: add(43) # HALT
      elif code == "LET":
        if len(arr) == 2: exit("после %s должно быть хотя бы одно слово" % code)
        expr(" ".join(arr[2:])) # вроде одна строчка, а внутри целое королевство кода ;'-}}}
      elif code == "IF":
        yeah = False
        for i in range(2, len(arr)):
          if arr[i].upper() in ("REM", "INPUT", "READ", "OUTPUT", "WRITE", "PRINT", "END", "LET", "IF", "GOTO"):
            yeah = i
            break
        if not yeah: exit("не обнаружена команда срабатывания после IF-выражения")
        reg = expr(" ".join(arr[2:yeah]))
        
        save = err_prefix
        
        label = get_label(":cond")
        add(20, reg) # LOAD <reg>
        add(42, label) # JZ <label>
        handler(["%s ..." % arr[0]] + arr[yeah:])
        add(-1, label)
        
        err_prefix = save
      
      elif code == "GOTO":
        if len(arr) != 3: exit("после %s должно быть ровно 1 слово" % code)
        try: label = int(arr[2])
        except ValueError: exit("3-ое слово - не число")
        label = ":line_%s" % label
        try: labels[label]
        except KeyError: exit("метка %s с таким номером строки не найдена :/" % label)
        add(40, label) # JUMP <label>
      else: exit("2-ое слово не поддерживается Simple Basic'ом")
    handler(arr)
  
  err_prefix = None

  print("~" * 60)
  print("    И того:")
  print("Константы:", consts)
  print("Регистров:", len(regs), "|", regs)
  print("Код:", codes)

  state = (codes, regs, consts)
  mem, sa = linker(state)
  
  print("~" * 60)
  print_mem(mem)
  return mem, sa

code = """
10 REM Это комментарий
20 INPUT A
30 INPUT B
35 IF A * B == 0 GOTO 20
40 LET C=-5* A- -B * -10/A + B
50 PRINT C
60 IF C < 0 GOTO 20
70 END
""" # B - 5 * A - B / A * 10

# mem, sa = translator(code)
# CurPath = os.path.dirname(__file__)
# with open(os.path.join(CurPath, "translated.mem"), "wb") as file: file.write(b"".join(bytes((i & 255, i >> 8)) for i in mem))

def main():
  import optparse
  parser = optparse.OptionParser(usage="sbt.py <file_input_path.py> <file_output_path.mem>")
  parser.add_option("-f", "--for_sat", action="store_true")
  options, args = parser.parse_args(sys.argv) # добавляет опцию --help и --for_sat
  
  args = args[1:]
  if len(args) != 2: parser.error("Ожидалось 2 строки после sbt.py")
  src, dist = args
  d_dir = os.path.dirname(dist)
  if not os.path.exists(src): parser.error("❌ Не найден файл-источник: '%s'" % src)
  if d_dir and not os.path.exists(d_dir): parser.error("❌ Не обнаружена дирректория файла-результата: '%s/'" % d_dir)
  
  try:
    with open(src) as file: code = file.read()
  except:
    print("❌ Ошибка открытия файла-источника:\n%s" % traceback.format_exc())
    return
  
  try:
    mem, sa = translator(code)
    # на самом деле Simple Assembler и Binary генерируются одновременно не зависимо от друг-друга без помощи sat.py, а не сначала sa, а потом mem, но в принципе это спорная ситуация по сравнению с ТЗ, так что и так пойдёт ;'-}
    # надеюсь это было рекомендаций а не обязаловкой, иначе это будет выглядеть абсурдно, как я в sat.py запихиваю sa переменную и получаю абсолютно то же, что и в mem переменной ;'-}}}}}}}
  except:
    print("❌ Ошибка компиляции:\n%s" % traceback.format_exc())
    return
  
  try:
    if options.for_sat: bin, content = False, "\n".join(sa)
    else: bin, content = True, b"".join(bytes((i & 255, i >> 8)) for i in mem)
    
    with open(dist, "wb" if bin else "w") as file: file.write(content)
    print("✅ Файл '%s' успешно сохранён (как %s)" % (dist, "mem" if bin else "sa"))
  except: print("❌ Ошибка сохранения файла-результата:\n%s" % traceback.format_exc())

if __name__ == "__main__": main()
