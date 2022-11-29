degree=$1
file=$2

for i in $(seq 1 10)
do
    echo "test $i time"
    ./micro_test -l $degree -n 8 >> $file
done
