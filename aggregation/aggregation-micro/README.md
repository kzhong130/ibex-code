## How to build
```
cmake . -B build
cp -r rsa-keys/ ./build
cd build
make -j
```

## How to run
Run `./aggregator_micro_test -d LOG_DEGREE -t TIMES`.

+ `LOG_DEGREE` is the logarithm of the total groups of the histogram (e.g., set it
to 15 if the total group is $2^{15}$). If not specified, the `LOG_DEGREE` is set
to 15.

+ `TIMES` is the times that you want to run the micro benchmark tests, by
  default it's set to 10.

The program will output the averaged results of the running micro benchmark tests.