# Ibex: Privacy-preserving ad conversion tracking and bidding

This repository contains the codes of the paper "Ibex: Privacy-preserving ad conversion tracking and bidding" in CCS 2022.

## Setup
Please refer to [install.md](./install.md).

## Code organization
+ Private histogram
    + [aggregation/aggregation-micro](./aggregation/aggregation-micro/):
      microbenchmarks of private histogram aggregation protocol.
    + [aggregation/throughput](./aggregation/throughput/): evaluation of
      throughput of using Ibex's private histogram protocol and non-private
      method.

+ Oblivious bidding
    + [bidding/libs](./bidding/libs/): implementation of libraries used in the
      oblivious bidding protocol.
    + [bidding/microbench](./bidding/microbench/): microbenchmarks of oblivious
      bidding protocol.
    + [bidding/end-to-end](./bidding/end-to-end/): evaluation of end-to-end
      latency of oblivious bidding protocol and non-private method.

## Instruction for running
Please refer to the `README.md` files under each directory.