declare -a ips=(
                "127.0.0.1"
                # "54.177.31.123"
                # "54.193.108.137"
                )

port=6666
user_name=$1
log_elem=$2
pir_num=$3
launch_server_per_machine=$4

echo "log elem: $log_elem"
echo "lancuh $launch_server_per_machine on one server"

for ip in "${ips[@]}"
do
    port=6666
    echo "connect to $ip"
    cmd="ssh $user_name@$ip 'cd; cd floc/code/SealPIR/build; "
    for i in $(seq 1 $launch_server_per_machine)
    do
        cmd1="bash run-pir-servers-multi.sh $pir_num $port $log_elem & "
        cmd="$cmd$cmd1"
        let port++
    done
    cmd1="'&"
    cmd="$cmd$cmd1"
    echo $cmd
    eval $cmd
done