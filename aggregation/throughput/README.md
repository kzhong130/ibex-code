## Evaluation setting
We simulate the scenario where a server that serves a 1MB static web page and the
client issues HTTP requests to access the server's page either using Ibex's
private histogram protocol to include additional materials or directly exposing
the user's group id.

## How to run
+ On the server machine run `python server.py`.
+ On the client run `wrk -t1 -c1 -d300s -R1 -s [the lua file you choose]
  --latency [http://(server_ip):8000]`.
    + `g14`, `g15`, `g16`, `baseline` represents the file of HTTP requests for Ibex's
      private histogram with $2^{14}$, $2^{15}$, $2^{16}$ and directly sending
      group id in plaintext respectively.
+ The output from the client side will depict the CDF of response time from the server.