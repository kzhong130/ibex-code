#### Environments

We experiment on AWS instances with Ubuntu 20.04. Please refer to our paper for
more details.

#### Dependencies
To compile and run the codes, please install the following dependencies:
+ `sudo apt install make g++ m4 lzip build-essential libidn11-dev cmake`.
+ Openssl, we tried out with the
  [commit](https://github.com/openssl/openssl/tree/c87a7f31a3db97376d764583ad5ee4a76db2cbef).
+ SEAL, we tried out with version [4.0.0](https://github.com/microsoft/SEAL/tree/4.0.0).
+ [GMP](https://gmplib.org), we tried out with version 6.2.1.
+ [SealPIR](https://github.com/microsoft/SealPIR), we used a snapshot version of
  SealPIR with some modifications. The codes of SealPIR we used are included in
  [SealPIR directory](./SealPIR/) so you don't need to install it manually.
+ [emp-sh2pc](https://github.com/emp-toolkit/emp-sh2pc).
+ [wrk 2](https://github.com/giltene/wrk2).