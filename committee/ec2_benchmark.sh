#!/bin/bash

PLAYER=$1
PROG_PATH=$2


reqs=$(./compile.sh $PROG_PATH | grep "Program requires:")

echo $reqs

N_TRIPLES=$(echo $reqs | grep -o \'triple\'\)..\[0-9\]* | grep -o \[0-9\]*)
N_BITS=$(echo $reqs | grep -o \'bit\'\)..\[0-9\]* | grep -o \[0-9\]*)
N_SQUARES=$(echo $reqs | grep -o \'square\'\)..\[0-9\]* | grep -o \[0-9\]*)

N_TRIPLES_INF=$(echo $reqs | grep "('modp', 'triple'): inf")
N_BITS_INF=$(echo $reqs | grep "('modp', 'bit'): inf")
N_SQUARES_INF=$(echo $reqs | grep "('modp', 'square'): inf")

if [[ $N_TRIPLES == '' ]]
then
  if [[ $N_TRIPLES_INF == '' ]]
  then
    N_TRIPLES=1  # 1 instead of 0 since 0 represents infinity
  else
    N_TRIPLES=0  # Produce an unlimited number
  fi
fi

if [[ $N_BITS == '' ]]
then
  if [[ $N_BITS_INF == '' ]]
  then
    N_BITS=1  # 1 instead of 0 since 0 represents infinity
  else
    N_BITS=0  # Produce an unlimited number
  fi
fi

if [[ $N_SQUARES == '' ]]
then
  if [[ $N_SQUARES_INF == '' ]]
  then
    N_SQUARES=1  # 1 instead of 0 since 0 represents infinity
  else
    N_SQUARES=0  # Produce an unlimited number
  fi
fi

echo
echo 'Measuring the runtime and communication cost of' $PROG_PATH

COMM_T0_LOCAL=$(cat /proc/net/dev | grep -o lo..\[0-9\]* | grep -o \[0-9\]*$)

if [[ ${COMM_T0_LOCAL} == '' ]]
then
  COMM_T0_LOCAL=0
fi

COMM_T0_ETH=$(cat /proc/net/dev | grep -o eth0..\[0-9\]* | grep -o \[0-9\]*$)

if [[ ${COMM_T0_ETH} == '' ]]
then
  COMM_T0_ETH=0
fi

COMM_T0=$(( ${COMM_T0_LOCAL} + ${COMM_T0_ETH} ))

T0=$(date +%s)

### Note: need to set corrent `portnums` here when changing number of players
./Player.x $PLAYER $PROG_PATH -portnums 5000,5001,5002,5003,5004
#> output_${PLAYER}.txt 2> err_${PLAYER}.txt
wait;

T1=$(date +%s)

COMM_T1_LOCAL=$(cat /proc/net/dev | grep -o lo..\[0-9\]* | grep -o \[0-9\]*$)

if [[ ${COMM_T1_LOCAL} == '' ]]
then
  COMM_T1_LOCAL=0
fi

COMM_T1_ETH=$(cat /proc/net/dev | grep -o eth0..\[0-9\]* | grep -o \[0-9\]*$)

if [[ ${COMM_T1_ETH} == '' ]]
then
  COMM_T1_ETH=0
fi


COMM_T1=$(( ${COMM_T1_LOCAL} + ${COMM_T1_ETH} ))

echo 'Run time (s):' $(( ${T1} - ${T0} )) > communication.txt
echo 'Communication Cost (bytes):' $((${COMM_T1} - ${COMM_T0})) >> communication.txt