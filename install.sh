### apt update
sudo apt-get update
sudo apt install make g++ m4 lzip build-essential libidn11-dev cmake

### install gmp lib
wget https://gmplib.org/download/gmp/gmp-6.2.1.tar.lz
lzip -d gmp-6.2.1.tar.lz
tar -xvf gmp-6.2.1.tar
cd gmp-6.2.1/
./configure
make
# make check
sudo make install

### install openssl
cd
echo "install opensll"
git clone https://github.com/openssl/openssl.git
cd openssl
git checkout c87a7f31a3db97376d764583ad5ee4a76db2cbef
./Configure
make
# make test
sudo make install

### remove old libssl-dev
sudo apt remove libssl-dev

### install SEAL 4.0.0
cd
git clone https://github.com/microsoft/SEAL.git
cd SEAL
git checkout 4.0.0
cmake -S . -B build
cmake --build build
sudo cmake --install build

### install emp
wget https://raw.githubusercontent.com/emp-toolkit/emp-readme/master/scripts/install.py
python3 install.py --deps --tool --ot --sh2pc

### refresh shared library cache
sudo ldconfig