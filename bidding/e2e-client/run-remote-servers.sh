declare -a ips=(
                "54.219.163.248"
                "54.177.31.123"
                # "54.193.108.137"
                )

user_name=$1
log_elem=$2
pir_num=$3

echo "log elem: $log_elem"

for ip in "${ips[@]}"
do
    echo "connect to $ip"
    eval "ssh $user_name@$ip                   \\
    'cd;                                    \\
    cd floc/code/SealPIR/build;             \\
    bash run-pir-servers.sh $pir_num 6666 $log_elem;       \\
    bash run-pir-servers.sh $pir_num 6766 $log_elem;       \\
    bash run-pir-servers.sh $pir_num 6866 $log_elem;      \\
    bash run-pir-servers.sh $pir_num 6966 $log_elem;      \\
    bash run-pir-servers.sh $pir_num 7066 $log_elem;      \\
    bash run-pir-servers.sh $pir_num 7166 $log_elem;      \\
    # bash run-pir-servers.sh $pir_num 8666 $log_elem;       \\
    # bash run-pir-servers.sh $pir_num 8766 $log_elem;       \\
    # bash run-pir-servers.sh $pir_num 8866 $log_elem;      \\
    # bash run-pir-servers.sh $pir_num 8966 $log_elem;      \\
    # bash run-pir-servers.sh $pir_num 9066 $log_elem;      \\
    # bash run-pir-servers.sh $pir_num 9166 $log_elem;      \\
    '&"
done