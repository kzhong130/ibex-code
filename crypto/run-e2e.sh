times=$1

echo "running test for 2^14"
echo "./server_e2e -b 15 -l 107 -p 148946578440312247013242417709057 -w 3 -n 48 -t $times"
./server_e2e -b 15 -l 107 -p 148946578440312247013242417709057 -w 3 -n 48 -t $times

echo "running test for 2^15"
echo "./server_e2e -n 48 -t $times"
./server_e2e -n 48 -t $times

echo "running test for 2^16"
echo "./server_e2e -b 17 -l 111 -p 1536920211333589944696312879382529 -w 3 -n 48 -t $times"
./server_e2e -b 17 -l 111 -p 1536920211333589944696312879382529 -w 3 -n 48 -t $times