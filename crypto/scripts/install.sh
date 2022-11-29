### apt update
sudo apt-get update
sudo apt install make g++ m4 lzip build-essential libidn11-dev cmake

# ### install cmake
# wget https://cmake.org/files/v3.4/cmake-3.4.0-Linux-x86_64.tar.gz
# tar -xzvf cmake-3.4.0-Linux-x86_64.tar.gz cmake-3.4.0-Linux-x86_64/
# sudo mv cmake-3.4.0-Linux-x86_64 /opt/cmake-3.4.0
# sudo ln -sf /opt/cmake-3.4.0/bin/*  /usr/bin/

### install gmp lib
wget https://gmplib.org/download/gmp/gmp-6.2.1.tar.lz
lzip -d gmp-6.2.1.tar.lz
tar -xvf gmp-6.2.1.tar
cd gmp-6.2.1/
./configure
make
make check
sudo make install

### install openssl
cd
echo "install opensll"
git clone https://github.com/openssl/openssl.git
cd openssl
git checkout c87a7f31a3db97376d764583ad5ee4a76db2cbef
./Configure
make
make test
sudo make install

### remove old libssl-dev
sudo apt remove libssl-dev

### refresh shared library cache
sudo ldconfig