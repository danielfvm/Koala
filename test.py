import random

def biggest(l):
    if len(l) <= 0:
        return 0

    b = l[0]
    bi = 0

    for i in range(0, len(l)-1):
        if l[i] > b:
            b = l[i]
            bi = i

    return bi

def sort(l):
    nl = []
    bi = 0

    for i in range(0, len(l)-1):
        bi = biggest(l)
        nl.append(l[bi])
        l[bi] = 0
    return nl

ml = [random.randint(0, 1000) for iter in range(1000)]

print(sort(ml))
