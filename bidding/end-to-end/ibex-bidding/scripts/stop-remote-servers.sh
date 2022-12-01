declare -a ips=(
                "54.219.163.248"
                "54.177.31.123"
                )

user_name=$1

for ip in "${ips[@]}"
do
    eval "ssh $user_name@$ip                   \\
    'pkill bidder;'                     \\
    &"
done