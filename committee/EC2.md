### Rough instructions to run on EC2 with 10 machines:

- Create 10 xlarge instances. Make sure to configure your security settings to allow incoming traffic on at least ports 5000-5000+N-1, where N is the number of machines

- SSH into each instance. On each node, do the following:

`sudo yum install docker`
`sudo yum install git`

`sudo service docker start`

`git clone https://github.com/karannewatia/Mycelium.git`

`cd Mycelium/crypto/code/scale_mamba_version`

`sudo docker build -t mpc . `

Set bash variables i=1 to i=10 for each corresponding node

`port_id=$(( 5000 + ${i} - 1))`

`sudo docker run -id -p ${port_id}:${port_id} mpc `

(This will work even if you close the docker and need  to run it again:
`sudo docker ps`
Grab the ID and run `sudo docker exec -ti [ID] bash`)

`cd SCALE-MAMBA`

`curl ifconfig.me` and store the resulting IP address

In this folder, create a cert.sh file as follows, with the IP addresses filled in:

```
echo 1
echo RootCA
echo 5
echo IP0
echo Player0.crt
echo IP1
echo Player1.crt
echo IP2
echo Player2.crt
echo IP3
echo Player3.crt
echo IP4
echo Player4.crt
```

For each node $i, change IP$i to 127.0.0.1

Run:

`cp Auto-Test-Data/1/* Data/`

`./cert.sh | ./Setup.x`

`./Setup.x` with inputs: 2 -> 2 -> p -> 4 (p listed below)

p = 148946578440312247013242417709057 (for 2^{15})

p = 558127740940706268294329795608577 (for 2^{16})

p = 1536920211333589944696312879382529 (for 2^{17})

Set bash variable i=0 -> 9 for corresponding machines

Running computation:

Copy files input.sh and ec2_benchmark.sh into this folder.

Run keygen first:

```./ec2_benchmark.sh $i Programs/secret_keygen```

Then setup and run decrypt (change `d` to the polynomial degree (e.g., 15) you experiment with):
```
python3 dec_input_gen.py ${d} > ~/SCALE-MAMBA/Data/Player${i}_in.txt
cp ~/SCALE-MAMBA/Data/Player${i}_shareout.txt ~/SCALE-MAMBA/Data/Player${i}_sharein.txt
./ec2_benchmark.sh $i Programs/dec_test
```


Timing and communication costs will be output to the command line.
