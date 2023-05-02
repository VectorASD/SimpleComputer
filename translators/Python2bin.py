import os

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
reversemap = {v : k for k, v in opmap.items()}

def get_name(num):
  n2s = grammar.number2symbol
  tn = tok_name
  return reversemap.get(num, tn[num] if num < grammar.start else n2s[num])

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
    return driver.parse_string(code)
  except ParseError as e:
    #print(e.msg)
    #print("%s '%s'" % (e.type, e.value), len(e.value))
    #print(e.context)
    RaiseSE(code, e.context, e.msg)

def compiler(code):
  tree = Parser(code)
  Recurs(tree)
  print(tree)

code = """
# Это комментарий
while True:
  C = input() - input()
  if C >= 0: break
print(C)
"""
compiler(code)
