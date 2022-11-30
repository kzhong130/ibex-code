bidder_num=$1
port=6666

for i in $(seq 1 $bidder_num)
do
    cmd1="./bidder 0.0.0.0:$port & "
    cmd="$cmd$cmd1"
    let port++
done

echo $cmd
eval $cmd
