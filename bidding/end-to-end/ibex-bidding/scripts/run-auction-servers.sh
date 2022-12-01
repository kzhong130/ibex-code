ip1=54.193.230.6
ip2=44.234.252.43
user_name=$1
bidder_num=$2
auctioneer_bin_path=$3

echo "bidder num: $bidder_num"

echo "connect to $ip1"
eval "ssh $user_name@$ip1                   \\
    'cd;                                    \\
    cd $auctioneer_bin_path;             \\
    ./bin/auctioneer 1 12345 $ip1 0.0.0.0:12346 $bidder_num;'       \\
    &"

echo "connect to $ip2"
eval "ssh $user_name@$ip2                   \\
    'cd;                                    \\
    cd $auctioneer_bin_path;             \\
    ./bin/auctioneer 2 12345 $ip1 0.0.0.0:12346 $bidder_num;'       \\
    &"
