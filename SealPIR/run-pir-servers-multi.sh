server_num=$1
port=$2
log_elem=$3
cmd=""

cmd1="./pir_servers -i 0.0.0.0:$port -n $server_num -l $log_elem > $port.txt & "
cmd="$cmd$cmd1"

echo $cmd
eval $cmd