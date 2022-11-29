#include <string.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "paillier.c"
#include "tools.c"

using namespace std;
using namespace chrono;

void enc(vector<mpz_t>& ciphertext, vector<mpz_t>& feature,
         paillier_public_key* pub_key, int begin, int end) {
    for (int idx = begin; idx < end; idx++) {
        mpz_init(ciphertext[idx]);
        paillier_encrypt(ciphertext[idx], feature[idx], pub_key);
    }
}

int main() {
    system_clock::time_point starttime, endtime;

    int vector_length = (1 << 16);
    int parallel_num = 8;
    vector<mpz_t> ciphertext = vector<mpz_t>(vector_length);
    vector<mpz_t> feature = vector<mpz_t>(vector_length);

    // Read public key
    paillier_public_key* pub_key = new paillier_public_key();
    FILE* pub_key_file = fopen("public.key", "rb");
    paillier_public_init(pub_key);
    paillier_public_in_str(pub_key, pub_key_file);
    fclose(pub_key_file);

    // Init feature vector
    gmp_randstate_t gmp_state;
    gmp_randinit_mt(gmp_state);

    for (int idx = 0; idx < vector_length; idx++) {
        mpz_init(feature[idx]);
        mpz_urandomb(feature[idx], gmp_state, 20);
    }

    // Encrypt plaintext vector into ciphertext
    starttime = system_clock::now();
    vector<thread> t_vec;

    // for (int idx = 0; idx < vector_length; idx++) {
    //     mpz_init(ciphertext[idx]);
    //     paillier_encrypt(ciphertext[idx], feature[idx], pub_key);
    // }

    for (int i = 0; i < parallel_num; i++) {
        thread t(&enc, ref(ciphertext), ref(feature), pub_key,
                 vector_length / parallel_num * i,
                 vector_length / parallel_num * (i + 1));
        t_vec.push_back(move(t));
    }

    for (auto& th : t_vec) {
        th.join();
    }
    endtime = system_clock::now();
    cout << "Encryption time: "
         << duration_cast<duration<double>>(endtime - starttime).count()
         << endl;

    mpz_t product;
    mpz_init(product);
    starttime = system_clock::now();
    for (int idx = 0; idx < vector_length; idx++) {
        paillier_homomorphic_add(product, product, ciphertext[idx], pub_key);
    }
    endtime = system_clock::now();
    cout << "aggregation time: "
         << duration_cast<duration<double>>(endtime - starttime).count()
         << endl;

    starttime = system_clock::now();
    vector<mpz_t> rotate(vector_length);
    for (int i = 0; i < vector_length; i++) {
        int idx = (i + 3) % vector_length;
        mpz_init(rotate[i]);
        mpz_set(rotate[i], ciphertext[idx]);
    }
    endtime = system_clock::now();
    cout << "shift time: "
         << duration_cast<duration<double>>(endtime - starttime).count()
         << endl;

    FILE* ctxt_file = fopen(string("ciphertext.bin").c_str(), "wb");
    for (int idx = 0; idx < vector_length; idx++) {
        mpz_out_raw(ctxt_file, ciphertext[idx]);
        mpz_clear(feature[idx]);
        mpz_clear(ciphertext[idx]);
    }
    fclose(ctxt_file);
    paillier_public_clear(pub_key);

    return 0;
}