# A simple script for finding a lgP-bit prime, p, such that p-1 is divisible by 2**(lgM + 1)

import sympy
import random 
from sympy.ntheory.residue_ntheory import primitive_root
import sys

lgP = int(sys.argv[1])

p = 4

lgM = int(sys.argv[2])

print("lgP", lgP)
print("lgM", lgM)

while (not sympy.isprime(p)):
  p = random.randrange(2**(lgP-lgM-2), 2**(lgP-lgM-1)-1) * 2**(lgM+1) +1

print("p", p)
print("w", primitive_root(p))