### Baseline of aggregation using [emp-sh2pc](https://github.com/emp-toolkit/emp-sh2pc)

We tried out with this commit (ddb557b00cd27efe2864b0adc9e85bc1dc78c1c0).

In this baseline, we assume there is one aux aggregator and one main aggregator,
they each hold one additive share of aggregation feature (e.g., cohort id).

Note that aux aggregator needs to work together with the main aggregator, that
is it can observe which report shares are used by the main aggregator. 
For example, it indicates that a user visits certain main aggregator's (e.g.,
advertiser) website, which can be used as cross-tracking by the aux aggregator.

To prevent cross-tracking, we implement the baseline such that the aux aggregator
supports aggregation for `many` main aggregators. During aggregation, the main
aggregator inputs an `index` of the target aggregation share to the 2pc, and
only aggregates the result of the `index`-th report into the main aggregator's
aggregation result.

### How to run it
./bin/test_hist 1 12345 & ./bin/test_hist 2 12345