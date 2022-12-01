declare -a ips=(
                "54.193.230.6"
                "44.234.252.43"
                )

user_name=$1

for ip in "${ips[@]}"
do
    eval "ssh $user_name@$ip                   \\
    'pkill auctioneer;'                     \\
    &"
done