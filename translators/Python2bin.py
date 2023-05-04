import os
import sys
import traceback

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
-1.) :<marker_name>

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
41.) JNEG <куда прыгнуть, если в аккуме отриц. число>
42.) JZ <куда прыгнуть, если в аккуме 0>
43.) HALT

71.) MOVA <адрес> | mem[accum] = mem[<адрес>]
72.) MOVR <адрес> | mem[<адрес>] = mem[accum]
"""

def printer(codes):
  for line in codes:
    code, first = line[0], line[1] if len(line) > 1 else "x"
    alt = "☘️ %s☘️ " % first if type(first) is str else first
    if code == -1: print(first, "~" * 5)
    elif code == 10: print("READ %s" % alt)
    elif code == 11: print("WRITE %s" % alt)
    elif code == 20: print("LOAD %s" % alt)
    elif code == 21: print("STORE %s" % alt)
    elif code == 30: print("ADD %s" % alt)
    elif code == 31: print("SUB %s" % alt)
    elif code == 32: print("DIVIDE %s" % alt)
    elif code == 33: print("MUL %s" % alt)
    elif code == 34: print("MOD %s" % alt)
    elif code == 40: print("JUMP %s" % alt)
    elif code == 41: print("JNEG %s" % alt)
    elif code == 42: print("JZ %s" % alt)
    elif code == 43: print("HALT")
    elif code == 71: print("MOVA %s" % alt)
    elif code == 72: print("MOVR %s" % alt)
    else: exit("printer: %s код не найден" % code)

to_sa = {
  10: "READ", 11: "WRITE",
  
  20: "LOAD", 21: "STORE",
  
  30: "ADD", 31: "SUB", 32: "DIVIDE", 
  33: "MUL", 34: "MOD",
  
  40: "JUMP", 41: "JNEG", 42: "JZ", 43: "HALT",
  
  71: "MOVA", 72: "MOVR",
}

def linker(state):
  codes, regs, consts = state

  # простенький оптимизатор LOAD и STORE
  in_accum, new, yeah = set(), [], 0 # in_accum - множество ячеек памяти, гарантирующие, что внутри них тоже, что и в аккумуляторе
  for op in codes:
    code, first = op[0], op[1] if len(op) > 1 else "x"
    if code == -1: in_accum.clear() # метка
    elif code == 10: in_accum.discard(first) # READ
    elif code == 11: pass # WRITE
    elif code == 20: # LOAD
      if first in in_accum:
        print("❌ ", end=""); printer([op])
        yeah += 1; continue
      in_accum.clear()
      in_accum.add(first)
    elif code == 21: # STORE
      if first in in_accum:
        print("❌ ", end=""); printer([op])
        yeah += 1;continue
      in_accum.add(first)
    elif code in (30, 31, 32, 33, 34): in_accum.clear() # ADD/SUB/DIVIDE/MUL/MOD
    elif code in (40, 41, 42, 43): pass # JUMP/JNEG/JZ/HALT - не влияют на содержимое аккумулятора в случае НЕсрабатывания
    elif code == 71: in_accum.clear() # MOVA
    else: in_accum.discard(first) # MOVR
    new.append(op)
    print("✅ ", end=""); printer([op])
  codes = new
  
  # обработка меток:
  new, links = [], {}
  for op in codes:
    if op[0] == -1: links[op[1]] = len(new)
    else: new.append(op)
  codes = new
  
  def encode(code, value):
    a, b = divmod(code, 10) # для удобства команды уже в 16-ричной системе счисления, т.е. 40 команда записывается, как 0x40vv
    return (a % 10) << 11 | b << 7 | (value & 0x7f)
  def linking(s):
    if type(s) is int: return s
    pref = s[0]
    if pref == ":": return links[s] + start_p
    num = int(s[1:])
    if pref == "c": return start_c + num
    if pref == "r": return start_r + num
    exit("Нет такого префикса: %s" % pref)

  limit = 100
  start_c = 1 # consts
  start_r = start_c + len(consts) # regs
  start_p = start_r + len(regs) # prog
  need = start_p + len(codes)
  print("✅✅✅ За счёт оптимизатора занято %s ячеек, но без него было бы %s (-%s)" % (need, need + yeah, yeah))
  if need > limit: exit("linker: УПС! Требуется %s ячеек памяти, а доступно %s" % (need, limit)) # Врагу не пожелаешь встретиться с этой ошибкой ;'-}
  
  mem = [0x4000] * limit
  mem[0] = encode(40, start_p)
  sa = ["00 JUMP   %02d ; (лаунчер)" % start_p]
  for n, const in enumerate(consts, start_c):
    res = 0 if const is None else const & 0x7fff
    mem[n] = res
    sa.append("%02d  =  %s%02x%02x ; (%s)" % (n, "+" if res < 0x4000 else "-", res >> 7 & 127, res & 127, "var" if const is None else "const"))
  for n in range(start_r, start_p):
    mem[n] = 0
    sa.append("%02d  =  +0000 ; (регистр)" % n)
  for n, code in enumerate(codes, start_p):
    if len(code) == 1:
      a, b = code[0], 0
    else:
      a, b = code
      b = code[1] = linking(b)
    res = encode(a, b)
    mem[n] = res
    sa.append("%02d %-6s %02d" % (n, to_sa[a], b) + (" ; (начало программы)" if n == start_p else ""))

  print("~" * 60)
  print(*sa, sep="\n")
  
  return mem, sa

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
    if n == -1: return
    if n[0] == "r":
      n = int(n[1:])
      if n in range(len(regs)): regs[n] = False
  vars = {} # накопитель переменных (пара: номер_константы, является_ли_массивом)
  def new_var(name):
    try: return vars[name][0]
    except KeyError: var = vars[name] = (new_const(), False)
    return var[0]
  def new_arr(name, len):
    try:
      a, b = vars[name]
      if not b: error("new_arr: Нельзя работать с обычной переменной '%s' по индексу. Размер массива заранее объявляется в коде" % name)
      return a
    except KeyError: pass
    if len is None: error("new_arr: Нигде не объявлен массив '%s' во время присвоения в его элемент чего-либо" % name)
    if type(len) is not int: error("new_arr: В качестве размера нового массива должно быть число, не переменная")
    c_arr = [new_const() for i in range(len)]
    res = c_arr[0]
    vars[name] = (res, True)
    return -1
  labels = {} # накопитель меток
  def get_label(name):
    count = labels.get(name, 0)
    labels[name] = count + 1
    return ":" + name + "_" + hex(count)[2:]
  loop_stack = [("!", "!")] # накопитель пар меток петель (петли: while, for)
  
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

  expr_types = ("NUMBER", "NAME", "arith_expr", "term", "print_stmt", "power", "atom", "comparison", "test")
  def expr(node):
    name = get_name(node)
    if name == "NUMBER":
      num = int(node.value)
      reg = new_const(num)
    elif name == "NAME":
      value = node.value
      if value == "pass": return -1
      if value == "return":
        if not codes or codes[-1][0] != 43: add(43) # HALT
        return -1
      if value in ("break", "continue"):
        label = loop_stack[-1][int(value == "continue")]
        if label == "!": RaiseSE(code, node, "'%s' outside loop" % node.value)
        add(40, label) # JUMP <label>
        return -1
      
      if value == "True": reg = new_const(1)
      elif value == "False": reg = new_const(0)
      else: reg = new_var(value)
    elif name in ("arith_expr", "term", "comparison"): # arith_expr это '+' и '-';  term это '*', '/' и '%';   comparison это '==', '!=', '<>', '>=', '>', '<=' и '<'
      cmp = name == "comparison"
      # Recurs(node)
      for n, node in enumerate(node.children):
        if n % 2:
          let = node
          if let.value == "<>": let.value = "!="
          continue
        if n == 0:
          reg = expr(node)
          add(20, reg) # LOAD <reg>
          continue
        reg2 = expr(node)
        value = let.value
        if cmp:
          label = get_label("cmp") # comparison
          label2 = get_label("cmpd") # comparison dropper
          # решил пойти по тяжёлому пути, а не добавлять новые 6 команд, как это положено во всех виртуалках во время разработки их кода
          add(31, reg2) # SUB <reg2>
          if value in ("==", "!="): # 5 операций вместо 1 :/
            # A == B -> A - B == 0 -> A - B == 0 ? 1 : 0
            # A != B -> A - B != 0 -> A - B == 0 ? 0 : 1
            add(42, label) # JZ <label>
            add(20, new_const(int(value == "!="))) # LOAD (0 при '==', либо 1 при '!=')
            add(40, label2) # JUMP <label2>
            add(-1, label)
            add(20, new_const(int(value == "=="))) # LOAD (1 при '==', либо 0 при '!=')
          elif value in ("<", ">="): # 5 операций вместо 1 :/
            # A < B -> A - B < 0 -> A - B < 0 ? 1 : 0
            # A >= B -> A - B >= 0 -> A - B < 0 ? 0 : 1
            add(41, label) # JNEG <label>
            add(20, new_const(int(value == ">="))) # LOAD (0 при '<', либо 1 при '>=')
            add(40, label2) # JUMP <label2>
            add(-1, label)
            add(20, new_const(int(value == "<"))) # LOAD (1 при '<', либо 0 при '>=')
          elif value in (">", "<="): # 6 операций вместо 1 :/
            # A > B -> A - B > 0 -> B - A < 0 ? 1 : 0
            # A <= B -> A - B <= 0 -> B - A < 0 ? 0 : 1
            # Слишком трудоёмко переворачивать цепочку операций, т.к. кучу LOAD'ов и STORE'ов придётся докинуть... Вот альтернатива:
            # A > B -> A - B > 0 -> A - B < 0 or A - B == 0 ? 0 : 1
            # A <= B -> A - B <= 0 -> A - B < 0 or A - B == 0 ? 1 : 0
            add(41, label) # JNEG <label>
            add(42, label) # JZ <label>
            add(20, new_const(int(value == ">"))) # LOAD (1 при '>', либо 0 при '<=')
            add(40, label2) # JUMP <label2>
            add(-1, label)
            add(20, new_const(int(value == "<="))) # LOAD (0 при '>', либо 1 при '<=')
          else: error("expr:comparison: пока не поддерживается '%s' операция" % value)
          add(-1, label2)
        else: add(augassign.index(value + "=") + 30, reg2) # ADD/SUB/DIVIDE/MUL/MOD <reg2>
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
      a, b = check_len("expr:power", node, 2) # цепочка из trailer'ов не поддерживается
      check_name("expr:power", a)
      check_name("expr:power", b, "trailer")
      childs = check_len("expr:power", b, (2, 3))
      if len(childs) == 3: d, e, f = childs
      else: d, e, f = childs[0], None, childs[1]
      name = get_name(d)
      if name == "LPAR":
        check_name("expr:power:(...)", f, ")")
        if a.value != "input": error("expr:power:(...): Поддерживается только метод input")
        if e is not None: error("expr:power:(...): У функции input() не должно быть аргументов")
        reg = new_reg()
        add(10, reg) # READ <reg>
      elif name == "LSQB":
        check_name("expr:power:[...]", f, "]")
        index = int(e.value) if get_name(e) == "NUMBER" else expr(e)
        reg = new_arr(a.value, index)
        if reg != -1:
          if reg[0] != "c": error("expr:power:[...]: ожидалась константа, а не %s" % reg)
          pos = int(reg[1:]) + 1
          if type(index) is int:
            add(20, new_const(pos + index)) # LOAD <c<pos+index>>
          else:
            add(20, new_const(pos)) # LOAD <c<pos>>
            add(30, index)
          reg = new_reg()
          add(72, reg) # MOVR <reg>
      else: error("expr: trailer: Ожидался LSQB или LPAR, а встречен", name)
    elif name == "atom":
      a, b, c = check_len("expr:atom", node, 3)
      check_name("expr:atom", a, "(")
      check_name("expr:atom", c, ")")
      check_name("expr:atom", b, expr_types)
      reg = expr(b)
    elif name == "factor":
      # Recurs(node)
      a, b = check_len("expr:factor", node, 2)
      name = check_name("expr:factor", a, ("PLUS", "MINUS", "TILDE"))
      is_num = get_name(b) == "NUMBER"
      # print(name, is_num)
      if name == "PLUS": reg = expr(b)
      elif name == "MINUS":
        if is_num: reg = new_const(-int(b.value)) # reg = expr(Leaf(2, "-" + b.value))
        else:
          reg = expr(b)
          add(20, new_const(0)) # LOAD 0
          add(31, reg) # SUB <reg>
          add(21, reg) # STORE <reg>
      else: # name == "TILDE"
        # reg = expr(b)
        # add(53, reg)
        error("Не поддерживается унарная тильда")
    elif name == "test":
      # Recurs(node)
      a, b, c, d, e = check_len("expr:test", node, 5)
      check_value("expr:test", b, "if")
      check_value("expr:test", d, "else")
      label = get_label("cond")
      label2 = get_label("goto")
      reg = expr(c)
      add(20, reg) # LOAD <reg>
      add(42, label) # JZ <label>
      free_reg(reg)
      reg = expr(a)
      free_reg(reg)
      reg2 = new_reg()
      if reg != reg2:
        add(20, reg) # LOAD <reg>
        add(21, reg2) # STORE <reg2>
      add(40, label2) # JUMP <label2>
      add(-1, label)
      reg = expr(e)
      free_reg(reg)
      if reg != reg2:
        add(20, reg) # LOAD <reg>
        add(21, reg2) # STORE <reg2>
      add(-1, label2)
      reg = reg2
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
    name = check_name("left_expr_stmt", node, ("NAME", "power")) # пока поддерживаются NAME и power слева от =, +=, -= и т.д.
    if name == "NAME":
      dst = new_var(node.value)
      if let is None or let.value == "=":
        add(20, reg) # LOAD <reg>
        add(21, dst) # STORE <dst>
      else:
        add(20, dst) # LOAD <dst>
        add(augassign.index(let.value) + 30, reg) # ADD/SUB/DIVIDE/MUL/MOD <reg>
        add(21, dst) # STORE <dst>
    else: # name == "power"
      left_power(node, reg, let)
  def left_power(node, reg, let = None):
    Recurs(node)
    a, b = check_len("left_power", node, 2)
    check_name("left_power", a)
    check_name("left_power", b, "trailer")
    name = a.value
    reg2 = new_arr(name, None)
    if reg2[0] != "c": error("left_power: Ожидалась константа (указатель начала массива) во время присвоения элементу массива")
    pos = int(reg2[1:]) + 1
    
    a, b, c = check_len("left_power:trailer", b, 3)
    check_name("left_power:trailer", a, "[")
    check_name("left_power:trailer", c, "]")
    if get_name(b) == "NUMBER":
      index = int(b.value)
      add(20, new_const(pos + index)) # LOAD <c<pos+index>>
    else:
      index = expr(b)
      add(20, new_const(pos)) # LOAD <c<pos>>
      add(30, index) # ADD <index>
    if let is None or let.value == "=":
      add(71, reg) # MOVA <reg>
    else:
      reg2, reg3 = new_reg(), new_reg()
      add(72, reg3) # MOVR <reg3>
      add(21, reg2) # STORE <reg2>
      add(20, reg3) # LOAD <reg3>
      add(augassign.index(let.value) + 30, reg) # ADD/SUB/DIVIDE/MUL/MOD <reg>
      add(21, reg3) # STORE <reg3>
      add(20, reg2) # LOAD <reg2>
      add(71, reg3) # MOVA <reg3>
      free_reg(reg2); free_reg(reg3)
  
  def simple_stmt(node):
    for node in node.children:
      name = get_name(node)
      if name in ("NEWLINE", "SEMI"): continue
      if name == "expr_stmt": expr_stmt(node)
      elif name in expr_types:
        reg = expr(node)
        free_reg(reg)
      else: exit("simple_stmt: Встречен неизвестый элемент: " + name)
  
  def suiter(node, func):
    name = get_name(node)
    if name == "simple_stmt": simple_stmt(node)
    elif name == "suite": suit(node)
    else: error("suiter:%s:" % func, name)
  def suit(node):
    for node in node.children:
      name = get_name(node)
      if name in ("ENDMARKER", "NEWLINE", "INDENT", "DEDENT"): continue
      if name == "simple_stmt": simple_stmt(node)
      elif name == "while_stmt":
        a, b, c, d = check_len("suit:while_stmt", node, 4)
        check_value("suit:while_stmt", a, "while")
        check_name("suit:while_stmt", c, ":")
        loop = get_label("goto")
        label = get_label("cond") # conditional
        loop_stack.append((label, loop))
        add(-1, loop)
        reg = expr(b)
        free_reg(reg)
        add(20, reg) # LOAD <reg>
        add(42, label) # JZ <label>
        suiter(d, "while_stmt: Неизвестное тело цикла")
        add(40, loop) # JUMP <loop>
        add(-1, label)
        loop_stack.pop()
      elif name == "if_stmt":
        #if len(nodes) not in (4, 7): error("suit:if_stmt: Недопустимый размер:", len(nodes)) Не актуально из-за elif-промежуточек
        nodes = node.children
        check_value("suit:if_stmt", nodes[0], "if")
        check_name("suit:if_stmt", nodes[2], ":")
        #Recurs(node)
        #print("~" * 32, "if 💚 ")
        label, label2 = get_label("cond"), None
        reg = expr(nodes[1])
        free_reg(reg)
        add(20, reg) # LOAD <reg>
        add(42, label) # JZ <label>
        #print("~" * 24)
        suiter(nodes[3], "if_stmt: Неизвестное тело основного условия (элемента #1)")
        pos, el_n = 4, 2
        while pos < len(nodes):
          #print("~" * 24)
          v = check_value("suit:if_stmt", nodes[pos], ("elif", "else"))
          if label2 is None: label2 = get_label("goto")
          add(40, label2) # JUMP <label2>
          if v == "elif":
            if get_name(nodes[pos + 2]) != "COLON": error("suit: if_stmt: Ожидался ':' элемент #%s" % el_n)
            if label is None: error("suit:if_stmt: После 'else' встречен 'elif'") # На деле такого быть не может из-за SyntaxError
            add(-1, label)
            label = get_label("cond")
            reg = expr(nodes[pos + 1])
            free_reg(reg)
            add(20, reg) # LOAD <reg>
            add(42, label) # JZ <label>
            suiter(nodes[pos + 3], "if_stmt: Неизвестное тело промежуточного условия (элемента #%s)" % el_n)
            pos += 4
          else: # v == "else"
            if get_name(nodes[pos + 1]) != "COLON": error("suit: if_stmt: Ожидался ':' элемент #%s" % el_n)
            add(-1, label)
            label = None
            suiter(nodes[pos + 2], "if_stmt: Неизвестное тело конечного условия (элемента #%s)" % el_n)
            pos += 3
          el_n += 1
        if label is not None: add(-1, label)
        if label2 is not None: add(-1, label2)
        #print("~" * 32, "endif 💛 ")
      else: exit("suit: Встречен неизвестый элемент: " + name)
  
  if get_name(tree) != "file_input": exit("Ожидалось синтаксическое дерево")
  if tn[tree.children[-1].type] != "ENDMARKER": exit("В конце ожидался маркер конца")
  suit(tree)

  if not codes or codes[-1][0] != 43: add(43) # HALT
  
  print("~" * 60)
  print("    И того:")
  print("Константы:", consts)
  print("Регистров:", len(regs), "|", regs)
  print("Код:", codes)
  # printer(codes) Т.к. тоже самое (даже круче) выводит оптимизатор внутри linker'а
  
  state = (codes, regs, consts)
  mem, sa = linker(state)
  
  print("~" * 60)
  print_mem(mem)
  return mem, sa

# успешно прошедший код:
code = """
input()
print(input())
print input() # эффект того, что lib2to3 одновеременно комбинирует синтаксис 2 и 3 питона, но в меру
print 11
print(7)
"""

code2 = """
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

code3 = """
print(False); print(True)
A = input()
B = input()
print(A == B); print(A != B); print(A <> B)
print(A > B); print(A <= B)
print(A < B); print(A >= B)
"""

code4 = """
# Это комментарий
while True:
  C = input() - input() # без оптимизатора это бы не влезало ;'-}
  if C >= 0: break
print(C)

n = -10
while n <= 10:
  num = n - 4
  if num < 0: cmp = 100
  elif num == 0: cmp = 10
  else: cmp = 1
  num = n
  if num < 0: num = -num
  print(num * 1000 + cmp)
  n += 1
""" # целевой код достигнут!!! По сути, базовая часть курсовой выполнена, осталось реализовать команды своего варианта (массивы и 2 доп-команды)

code5 = """
arr[8] # таким образом можно дать знать компилятору, что в массиве 8 элементов ;'-} без объявления никак, объявить размер через переменную низя, динамика в 100 ячеек памяти не влезет по хорошему, про кучу/стек я вообще молчу ;'-}
arr = input()
print(arr[0])
i = 0
while i < 8:
  if i % 3 == 0: content = input()
  arr[i] = content
  i += 1
arr[0] += 10 # АААХАХХА! O_u 98/100 ячеек памяти стало забито, когда я добавил '+=', '*=' и '%=' над индексами массивов... 
arr[3] *= 3 # P.S. теперь 95/100, т.к. у меня утечка 3 регистров по ошибке была как раз из-за этих 3 операций ;'-} 
arr[6] %= 4 # P.P.S. Теперь 99/100, т.к. ещё кое-что фиксил 3 часа...
i = 7
while i >= 0:
  print(arr[i])
  i -= 1
""" # этот код абсолютно полностью доказывает работоспособность команд MOVA и MOVR! Есть некоторые неудобства, например, эти команды сохраняют и загружают в память, а не аккумулятор, но терпин ;)

code6 = """
i = 0
while True:
  num = i - 4
  print(-1 if num < 0 else 0 if num == 0 else 1)
  i += 1
  if i >= 10: break
"""

# разработка:
# пока-что нечего разрабатывать. В этом разделе и рождались code, code2, code3, code4, code5 и code6 переменные, но каждый раз, когда они полностью проходили проверку компиляцией и исполненем, то переходили в предыдущей раздел
# P.S. чисто для формальности вытащил все 6 блоков кода в файлы в папку "py/"

def main():
  import optparse
  parser = optparse.OptionParser(usage="Python2bin.py <file_input_path.py> <file_output_path.mem>")
  parser.add_option("-f", "--for_sat", action="store_true")
  options, args = parser.parse_args(sys.argv) # добавляет опцию --help и --for_sat
  
  args = args[1:]
  if len(args) != 2: parser.error("Ожидалось 2 строки после Python2bin.py")
  src, dist = args
  d_dir = os.path.dirname(dist)
  if not os.path.exists(src): parser.error("❌ Не найден файл-источник: '%s'" % src)
  if d_dir and not os.path.exists(d_dir): parser.error("❌ Не обнаружена дирректория файла-результата: '%s/'" % d_dir)
  
  try:
    with open(src) as file: code = file.read()
  except:
    print("❌ Ошибка открытия файла-источника:\n%s" % traceback.format_exc())
    return
  
  try: mem, sa = compiler(code)
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
