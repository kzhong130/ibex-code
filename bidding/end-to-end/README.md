# End-to-end evaluation of latency
This part compares the end-to-end latency of an auction from the browser sending
invitation requests to bidders until the auction server finishes computing the
auction.

The evaluation consists of the non-private baseline where all bids are sent in
clear to auction server and Ibex's oblivious bidding protocol.

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

## Ibex's oblivious bidding protocol
### How to build

### How to run