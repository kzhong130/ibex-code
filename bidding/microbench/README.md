## Micro benchmark for PIR
### How to build
```
cmake . -B build
cd build
make -j
```

### How to run
+ Run `./pir_micro -l [log of total groups]` (e.g., 15 for $2^{15}$ groups).

### Note
The `pir_micro.cpp` file is slightly modified from the original
[SealPIR](https://github.com/microsoft/SealPIR) repo.

## Micro benchmark for auction
### How to build
```
cmake . -B build
cd build
make -j
```

### How to run
+ Costs of local 2PC auction
    + Enter `build` directory.
    + Run `./bin/auction 0 5555 [bidder number] & ./bin/auction 1 5555 [bidder
      number]`.

+ Costs related to bidding shares
    + Copy `pk` and `sk` in [files](../files/) directory into `build/` directory.
    + Run `./bidding_share [bidder number]`.