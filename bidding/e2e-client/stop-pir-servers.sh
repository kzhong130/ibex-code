declare -a ips=(
                "127.0.0.1"
                # "54.177.31.123"
                # "54.193.108.137"
                )

user_name=$1

for ip in "${ips[@]}"
do
    eval "ssh $user_name@$ip                   \\
    'pkill pir_servers;'                     \\
    &"
done