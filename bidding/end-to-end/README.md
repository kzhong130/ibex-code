# End-to-end evaluation of latency
This part compares the end-to-end latency of an auction from the browser sending
invitation requests to bidders until the auction server finishes computing the
auction.

The evaluation consists of the non-private baseline where all bids are sent in
clear to auction server and Ibex's oblivious bidding protocol.

## Ibex's oblivious bidding protocol
### How to build
```
cmake . -B build
cd build
make -j
```

### How to run

#### Setup
+ Establish the SSH keyless connections from your `client` machine to all other
  machines of bidders and auction servers. Make sure you have connected once
  from your `client` machine to all other machines to establish authenticity.
+ On all other machines, build the programs correctly.

#### Edit config files
+ Edit the variable `ips` in `stop-bidders.sh` and
  `run-bidders.sh`, change it to the the ips of the bidders
  machine. We assume using `c5.24xlarge instances`, so each server simulate 6
  bidders.
+ Edit the variable `ips`, `ip1`, and `ip2` in `stop-auction-servers.sh` and
  `start-auction-servers.sh`, change it to the the ips of the two auction servers.
+ Copy all files in [files](../files/) directory into `build/` directory on
  all machines where the `bidder` and `client` programs exist.
+ Copy all scripts in [script](./ibex/scripts/) directory into `build/`
  directory on all machines where the `bidder` and `client` programs exist.

#### Start the bidders
+ On the client machine, run `bash run-bidders.sh [user name]
  [logarithm of total group] [path of bidder program]`.
  + `user name` is the user name you used to log into your bidders machine.
  + `logarithm of total group`, for example, is set to 15 for $2^{15}$ groups.
  + `[path of bidder program]` is the path on the bidder servers where the
    bidder program is. For example, it could be `ibex/bidding/end-to-end/ibex-bidding/build/`

#### Start the auction servers
+ On the client machine, run `bash run-auction-servers.sh [user name] [bidder
  number] [path of bidder program]`.
  + `user name` is the user name you used to log into your bidders machine.
  + `[path of bidder program]` is the path on the bidder servers where the
  auctioneer program is. For example, it could be
  `ibex/bidding/end-to-end/ibex-bidding/auctioneer/build/bin``.

#### Start the client
+ On the client machine, copy `ips.txt` into `build/` directory and modify
  `ips.txt` accordingly.
    + The format of `ips.txt` is `IP:PORT` in each line
    + The first two lines are ip addresses of the two auction servers.
    + Afterwards each line represents a bidder's PIR server.
+ On the client machine, run `./client -b [bidder number] -l [logarithm of total
  group]`.

#### Stop auctioneer and bidders
+ After each run, on the client machine, run `bash stop-auction-servers.sh [user
  name]` and `bash stop-bidders.sh [user name]`.

## Non-private baseline
### How to build
```
cmake . -B build
cd build
make -j
```

### How to run
+ Copy `cert.pem` and `key.pem` in [files](../files/) directory into `build` directory.
+ Each line in `ips.txt` represents a bidder, in the format of `IP:PORT`. The
  `auctioneer` program will read the IP addresses of bidders from this file, please
  change it accordingly and copy it into the `build` directory.
+ On the bidder machine, copy `start-bidders.sh` into the `build` directory. Run
  `bash start-bidders.sh [bidder number]` to start the bidders programs which
  starting listening from port `6666` to `6666+[bidder number]`.
+ On the auctioneer machine, run `./auctioneer [bidder number]` to start the
  auctioneer program and listening on the port 5555.
+ On the client machine, run `./client [auctioneer IP]`. `auctioneer IP` is in
  the format of `IP:PORT`. The `client` program will output the total latency.