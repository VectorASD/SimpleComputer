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

# целевой код достигнут!!! По сути, базовая часть курсовой выполнена, осталось реализовать команды своего варианта (массивы и 2 доп-команды)
