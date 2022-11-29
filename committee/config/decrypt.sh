#!/bin/bash

N_PLAYERS=$1
degree=$2

for (( i = 0; i < $N_PLAYERS; i++ ))
do
  python3 ../dec_input_gen.py $2 > Player${i}_in.txt
  cp ~/SCALE-MAMBA/Data/Player${i}_shareout.txt Player${i}_sharein.txt
done
