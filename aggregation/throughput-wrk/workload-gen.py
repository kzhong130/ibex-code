import os
import sys

# cohort size in MB, 0 means non-private baseline
cohort_len=float(sys.argv[1])
head = "wrk.method = \"POST\"\nwrk.headers[\"Content-Type\"] = \"application/json\"\nwrk.body=\"{"
template="\\\"tmp\\\":\\\"tmp\\\","
tail = "}\""
cohort=""

if cohort_len > 0:
    cohort= template*int(1024*1024*cohort_len/12)

print(head+cohort[:-1]+tail)
