import os
import sys

def exit(*args, **kw_args):
  print(*args, **kw_args)
  sys.exit()

import lib2to3
from lib2to3 import pytree
from lib2to3.pgen2 import driver as pgen2_driver
GPath = os.path.dirname(lib2to3.__file__)
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
"""

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

  const = [] # рассчитано только под числа
  def new_const(data):
    n = len(const)
    const.append(data)
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

  expr_types = ("NUMBER", "print_stmt", "power", "atom")
  def expr(node):
    name = get_name(node)
    # Recurs(node)
    if name == "NUMBER":
      num = int(node.value)
      reg = new_const(num)
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
  
  def simple_stmt(node):
    for node in node.children:
      name = get_name(node)
      if name in ("NEWLINE", "SEMI"): continue
      if name in expr_types:
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
  print("~" * 60)
  print("    И того:")
  print("Константы:", const)
  print("Регистров:", len(regs), "|", regs)
  print("Код:", codes)

code = """
# Это комментарий
while True:
  C = input() - input()
  if C >= 0: break
print(C)
"""

code = """
input()
print(input())
print input() # эффект того, что lib2to3 одновеременно комбинирует синтаксис 2 и 3 питона, но в меру
print 11
print(7)
"""
compiler(code)
