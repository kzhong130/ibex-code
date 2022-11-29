rm -r build
mkdir build
cd build
cmake ..
make -j8

rm ../paillier.txt
mkdir weight
mkdir feature
mkdir random
mkdir ciphertext
mkdir product
for count in {1..10}
    do
        ./key_generation >> ../paillier.txt
        ./encryption >> ../paillier.txt
        ./inner_product >> ../paillier.txt
        ./decryption >> ../paillier.txt

        du -sh ciphertext >> ../paillier.txt
        du -sh product >> ../paillier.txt
        ls -lh *.key >> ../paillier.txt
    done