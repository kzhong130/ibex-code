# Micro benchmark for auction
## How to build
```
cmake . -B build
cd build
make -j
```

## How to run
+ Costs of local 2PC auction
    + Enter `build` directory.
    + Run `./bin/auction 0 5555 [bidder number] & ./bin/auction 1 5555 [bidder
      number]`.

+ Costs related to bidding shares
    + Copy `pk` and `sk` in [files directory](../files/) into `build/bin` directory.
    + Run `./bidding_share [bidder number]`.