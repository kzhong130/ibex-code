### How to set up
Make sure that environments, programs, and related files (e.g., `*.pem`, `sk`,
`pk`) are put in the correct directory.

####

+ First generates a file `ips.txt` in the directory where `client` program is.
    + The format of `ips.txt` is `IP:PORT` in each line
    + The first two lines are ip address of the two auction servers.
    + Afterwards each line represents an advertiser's PIR server.
    + An example of `ips.txx` is given below
    ```
    128.110.219.46:12346
    128.110.219.61:12346
    128.110.219.46:6666
    128.110.219.46:6766
    128.110.219.46:6866
    128.110.219.61:6666
    128.110.219.61:6766
    128.110.219.61:6866
    ```
+ Edit the variable `ips` in `stop-remote-servers.sh` and
  `start-remote-servers.sh`, change it to the the ips of the advertisers
  machine. We assume using `c5.24xlarge instances`, so each server simulate 6
  advertisers.

+ Edit the variable `ips` in `stop-auction-servers.sh` and
  `start-auction-servers.sh`, change it to the the ips of the auction server
  machine.

+ Run `bash start-remote-servers.sh $USER_NAME $LOG_ELEM`.

+ Run `bash start-auction-servers.sh $USER_NAME $BIDDER_NUM`.
