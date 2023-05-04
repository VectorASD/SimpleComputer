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
