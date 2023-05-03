import os
import sys

def exit(*args, **kw_args):
  print(*args, **kw_args)
  sys.exit()

import lib2to3 # если у Вас не 3.10.4 версия питона и нет lib2to3, то вот решеньице: make fix_parser
from lib2to3 import pytree
from lib2to3.pgen2 import driver as pgen2_driver
GPath = os.path.dirname(lib2to3.__file__)
CurPath = os.path.dirname(__file__)
grammar = pgen2_driver.load_grammar(os.path.join(GPath, "Grammar.txt")) #, force = True)
driver = pgen2_driver.Driver(grammar, convert=pytree.convert)

from lib2to3.pytree import Node, Leaf
from lib2to3.pgen2.token import tok_name
from lib2to3.pgen2.parse import ParseError
from lib2to3.pgen2.grammar import opmap
# reversemap = {v : k for k, v in opmap.items()}
# def get_name(num):
#   n2s = grammar.number2symbol
#   tn = tok_name
#   return reversemap.get(num, tn[num] if num < grammar.start else n2s[num])

def Recurs(node, level = 0):
  leaf = isinstance(node, Leaf)
  if leaf:
    print("   |" * level + " %3s" % node.type, tok_name[node.type], repr(node.value))
  else:
    print("   |" * level + " %3s" % node.type, grammar.number2symbol[node.type])
    for i in node.children: Recurs(i, level + 1)

def RaiseSE(code, context, msg):
  if isinstance(context, Leaf): context = context._prefix, (context.lineno, context.column)
  prefix, (lineno, column) = context
  line = code.splitlines()[lineno - 1]
  while column >= len(line): column -= 1
  while column < len(line) - 1 and line[column] == " ": column += 1
  while line[0] == " ":
    line = line[1:]
    column -= 1
  print('  File "UwU.пудель", line', lineno)
  print("    " + line)
  print("    " + " " * column + "^")
  print("SyntaxError:", msg)
  exit()

def Parser(code):
  try:
    return driver.parse_string(code + "\n")
  except ParseError as e:
    #print(e.msg)
    #print("%s '%s'" % (e.type, e.value), len(e.value))
    #print(e.context)
    RaiseSE(code, e.context, e.msg)

augassign = ['+=', '-=', '/=', '*=', '%=']
"""
Основная модель результатирующего кода:
1 ячейка.) Всегда здесь будет располагаться JMP до программы
const_count.) Ячейки под константы
regs_count.) Ячейки под регистры
code_length.) Сама программа

Модель простая, как 5 пальцев ;'-}

Перечень кодов:
10.) READ <куда считываем>
11.) WRITE <что печатаем>

20.) LOAD <что загружаем в аккум>
21.) STORE <куда загружаем аккум>

30.) ADD <с чем складываем аккум>
31.) SUB <с чем вычитаем аккум>
32.) DIVIDE <с чем делим аккум>
33.) MUL <с чем умножаем аккум>
34.) MOD <с чем делим по модулю аккум>

40.) JUMP <куда прыгнуть>
43.) HALT
"""

def printer(codes):
  for line in codes:
    code, first = line[0], line[1] if len(line) > 1 else "x"
    alt = "☘️ %s☘️ " % first if type(first) is str else first
    if code == 10: print("READ %s" % alt)
    elif code == 11: print("WRITE %s" % alt)
    elif code == 20: print("LOAD %s" % alt)
    elif code == 21: print("STORE %s" % alt)
    elif code == 30: print("ADD %s" % alt)
    elif code == 31: print("SUB %s" % alt)
    elif code == 32: print("DIVIDE %s" % alt)
    elif code == 33: print("MUL %s" % alt)
    elif code == 34: print("MOD %s" % alt)
    elif code == 40: print("JUMP %s" % alt)
    elif code == 43: print("HALT")
    else: exit("printer: %s код не найден" % code)

def linker(state):
  def encode(code, value):
    a, b = divmod(code, 10) # для удобства команды уже в 16-ричной системе счисления, т.е. 40 команда записывается, как 0x40vv
    return (a % 10) << 11 | b << 7 | (value & 0x7f)
  def linking(s):
    if type(s) is int: return s
    pref, num = s[0], int(s[1:])
    if pref == "c": return start_c + num
    if pref == "r": return start_r + num
    exit("Нет такого префикса: %s" % pref)

  limit = 100
  codes, regs, consts = state
  start_c = 1 # consts
  start_r = start_c + len(consts) # regs
  start_p = start_r + len(regs) # prog
  need = start_p + len(codes)
  if need > limit: error("УПС! Требуется %s ячеек памяти, а доступно %s" % (need, state)) # Врагу не пожелаешь встретиться с этой ошибкой ;'-}

  mem = [0x4000] * limit
  mem[0] = encode(40, start_p)
  for n, const in enumerate(consts, start_c): mem[n] = 0 if const is None else const & 0x7fff
  for n in range(start_r, start_p): mem[n] = 0
  for n, code in enumerate(codes, start_p):
    if len(code) == 1:
      mem[n] = encode(code[0], 0)
      continue
    a, b = code
    b = code[1] = linking(b)
    mem[n] = encode(a, b)
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

def compiler(code):
  tree = Parser(code)
  Recurs(tree)
  print(tree)

  n2s = grammar.number2symbol
  tn = tok_name
  def get_name(node): return tn[node.type] if isinstance(node, Leaf) else n2s[node.type]
  def error(*args): exit("❌", *args, " ")
  codes = []
  def add(code, *data): codes.append([code] + list(data))

  consts = [] # рассчитано только под числа и переменные
  def new_const(data = None):
    if data is not None:
      try: return "c%s" % consts.index(data)
      except ValueError: pass
    n = len(consts)
    consts.append(data)
    return "c%s" % n
  regs = [] # регистры нужны для промежуточных операций
  def new_reg():
    for n, i in enumerate(regs):
      if not i:
        regs[n] = True
        return "r%s" % n
    regs.append(True)
    return "r%s" % (len(regs) - 1)
  def free_reg(n):
    if n[0] == "r":
      n = int(n[1:])
      if n in range(len(regs)): regs[n] = False
  vars = {} # накопитель переменных
  def new_var(name):
    try: return vars[name]
    except KeyError: var = vars[name] = new_const()
    return var
  
  def test_name(func, node, values = "NAME"):
    name = get_name(node)
    if type(values) is str: values = [values]
    for i, value in enumerate(values):
      value = opmap.get(value, value)
      if type(value) is int: values[i] = tok_name[value]
    if name not in values: return "~%s: Ожидался тип %s, но оказался '%s' внутри ноды:\n  %s" % (func, ", либо ".join("'%s'" % i for i in values), name, repr(node))
    return name
  def check_name(func, node, values = "NAME"):
    name = test_name(func, node, values)
    if name[0] == "~": error(name[1:])
    return name
  def test_value(node, value):
    if test_name("...", node)[0] == "~": return False
    return node.value == value
  def check_value(func, node, value):
    check_name(func, node)
    if type(value) not in (list, tuple): value = (value,)
    if node.value not in value: error("%s: Ожидалась величина %s, но оказалась '%s' внутри ноды:\n  %s" % (func, ", либо ".join("'%s'" % v for v in value), node.value, repr(node)))
    return node.value
  def check_len(func, node, lens):
    childs = node.children
    if type(lens) not in (list, tuple): lens = (lens,)
    if len(childs) not in lens: error("%s: Ожидался размер потомства = %s, но встречено %s внутри ноды:\n  %s" % (func, lens, len(childs), repr(node)))
    return childs

  expr_types = ("NUMBER", "NAME", "arith_expr", "term", "print_stmt", "power", "atom")
  def expr(node):
    name = get_name(node)
    if name == "NUMBER":
      num = int(node.value)
      reg = new_const(num)
    elif name == "NAME":
      reg = new_var(node.value)
    elif name in ("arith_expr", "term"): # arith_expr это '+' и '-';  term это '*', '/' и '%'
      # Recurs(node)
      for n, node in enumerate(node.children):
        if n % 2:
          let = node
          continue
        if n == 0:
          reg = expr(node)
          add(20, reg) # LOAD <reg>
          continue
        reg2 = expr(node)
        value = let.value
        add(augassign.index(value + "=") + 30, reg2) # ADD/SUB/DIVIDE/MUL/MOD <reg2>
        free_reg(reg2)
      if reg[0] == 'c': reg = new_reg()
      add(21, reg) # STORE <reg>
      # exit()
    elif name == "print_stmt":
      a, b = check_len("expr:print_stmt", node, 2)
      check_value("expr:print_stmt", a, "print")
      check_name("expr:print_stmt", b, expr_types)
      reg = expr(b)
      add(11, reg) # WRITE <reg>
    elif name == "power":
      a, b = check_len("expr:power", node, 2)
      check_name("expr:power", a)
      if a.value != "input": error("expr:power: Поддерживается только метод input")
      check_name("expr:power", b, "trailer")
      # пока не обрабатываю содержимое trailer, хоть это и может быть не только вызов функции, но и атрибут, либо обращение к элементу массива.
      reg = new_reg()
      add(10, reg) # READ <reg>
    elif name == "atom":
      a, b, c = check_len("expr:atom", node, 3)
      check_name("expr:atom", a, "(")
      check_name("expr:atom", c, ")")
      check_name("expr:atom", b, expr_types)
      reg = expr(b)
    else: error("expr: Не известный тип:", name)
    return reg
  
  def expr_stmt(node):
    if len(node.children) % 2 != 1: error("expr_stmt: Размер expr_stmt чётный и равен", len(node.children))
    right = node.children[-1]
    reg = expr(right)
    for n, node in enumerate(node.children[::-1][1:]):
      if n % 2 == 0: let = node
      else: left_expr_stmt(node, reg, let)
    free_reg(reg)
  def left_expr_stmt(node, reg, let = None):
    check_name("left_expr_stmt", node) # пока поддерживаются NAME слева от =, +=, -= и т.д.
    dst = new_var(node.value)
    if let is None or let.value == "=":
      add(20, reg) # LOAD <reg>
      add(21, dst) # STORE <dst>
    else:
      add(20, dst) # LOAD <dst>
      add(augassign.index(let.value) + 30, reg) # ADD/SUB/DIVIDE/MUL/MOD <reg>
      add(21, dst) # STORE <dst>
    
  def simple_stmt(node):
    for node in node.children:
      name = get_name(node)
      if name in ("NEWLINE", "SEMI"): continue
      if name == "expr_stmt": expr_stmt(node)
      elif name in expr_types:
        reg = expr(node)
        free_reg(reg)
      else: exit("simple_stmt: Встречен неизвестый элемент: " + name)
  
  def suit(node):
    for node in node.children:
      name = get_name(node)
      if name in ("ENDMARKER", "NEWLINE", "INDENT", "DEDENT"): continue
      if name == "simple_stmt": simple_stmt(node)
      else: exit("suit: Встречен неизвестый элемент: " + name)
  
  if get_name(tree) != "file_input": exit("Ожидалось синтаксическое дерево")
  if tn[tree.children[-1].type] != "ENDMARKER": exit("В конце ожидался маркер конца")
  suit(tree)

  if codes and codes[-1][0] != 43: add(43) # HALT
  
  print("~" * 60)
  print("    И того:")
  print("Константы:", consts)
  print("Регистров:", len(regs), "|", regs)
  print("Код:", codes)
  printer(codes)
  
  state = codes, regs, consts
  mem = linker(state)
  print("~" * 60)
  printer(codes)
  
  print("~" * 60)
  print_mem(mem)
  with open(os.path.join(CurPath, "compiled.mem"), "wb") as file: file.write(b"".join(bytes((i & 255, i >> 8)) for i in mem))

code = """
# Это комментарий
while True:
  C = input() - input()
  if C >= 0: break
print(C)
"""

# успешно прошедший код:
code = """
input()
print(input())
print input() # эффект того, что lib2to3 одновеременно комбинирует синтаксис 2 и 3 питона, но в меру
print 11
print(7)
"""

# разработка:
code = """
B; A; D # так можно манипулировать порядком "констант", что занимают эти переменные
A = input()
B = input()
print(B)
print(A)
print(C) # 0
print(D) # 0
A += 11; print(A)
B -= 5; print(B)
A *= 7; print(A)
A /= 3; print(A)
A %= 123; print(A)
C = (A + B + A) * 10 - 5
print(C / 123); print(C % 123)
"""

compiler(code)
