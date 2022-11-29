server_num=$1
port=$2
log_elem=$3
cmd=""

echo $t
for i in $(seq 1 $server_num)
do
    cmd1="./net_server -i 0.0.0.0:$port -n $server_num -l $log_elem > $port.txt & "
    cmd="$cmd$cmd1"
    let port++
done

echo $cmd
eval $cmd