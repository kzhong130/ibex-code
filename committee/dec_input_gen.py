import math
import sys

def mpc_dec_input(n):
    for i in range(3*(2**n)):
        #dummy ciphertext used for benchmarking the costs since the costs are input-oblivious
        print(1)

mpc_dec_input(int(sys.argv[1]))
