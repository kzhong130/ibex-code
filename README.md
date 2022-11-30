# Ibex: Privacy-preserving ad conversion tracking and bidding

This repository contains the codes of the paper "Ibex: Privacy-preserving ad conversion tracking and bidding" in CCS 2022.

## Setup
Please refer to [install.md](./install.md).

## Code organization
+ Private histogram
    + [aggregation/aggregation-micro](./aggregation/aggregation-micro/):
      implementation of microbenchmarks of private histogram aggregation
      protocol.
    + [aggregation/throughput](./aggregation/throughput/): Evaluation of
      throughput of using Ibex's private histogram protocol and non-private
      method.

+ Oblivious bidding