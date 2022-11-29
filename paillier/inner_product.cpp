#include <iostream>
#include <chrono>
#include "tools.c"
#include "paillier.c"

using namespace std;
using namespace chrono;

int main() {
    system_clock::time_point starttime, endtime;

    int vector_length = 1024;
    mpz_t ciphertext[vector_length], random[vector_length], rand_ctxt[vector_length], weight[vector_length], product[vector_length];

    // Read public key
    paillier_public_key* pub_key = new paillier_public_key();
    FILE* pub_key_file = fopen("public.key","rb");
	paillier_public_init(pub_key);
	paillier_public_in_str(pub_key, pub_key_file);
    fclose(pub_key_file);

    // Init weight and random vectors
    gmp_randstate_t gmp_state;
    gmp_randinit_mt(gmp_state);
	for (int idx = 0; idx < vector_length; idx++) {
        mpz_init(random[idx]);
        mpz_urandomb(random[idx], gmp_state, 20);
        mpz_init(weight[idx]);
        mpz_urandomb(weight[idx], gmp_state, 20);
    }
    for (int idx = 0; idx < vector_length; idx++) {
        mpz_init(ciphertext[idx]);
        FILE* ctxt_file = fopen((string("ciphertext/") + to_string(idx) + string(".bin")).c_str(),"rb");
        mpz_inp_raw(ciphertext[idx], ctxt_file);
        fclose(ctxt_file);
    }

    // Compute ctxt-ptxt multiplication and ctxt addition
    starttime = system_clock::now();
    for (int idx = 0; idx < vector_length; idx++) {
        mpz_init(product[idx]);
        mpz_init(rand_ctxt[idx]);
        paillier_homomorphic_multc(product[idx], ciphertext[idx], weight[idx], pub_key);
        paillier_encrypt(rand_ctxt[idx], random[idx], pub_key);
        paillier_homomorphic_add(product[idx], rand_ctxt[idx], product[idx], pub_key);
    }
    endtime = system_clock::now();
    cout << "Inner product computation time: " << duration_cast<duration<double>>(endtime - starttime).count() << endl;

	for (int idx = 0; idx < vector_length; idx++) {
        FILE* ptxt_file = fopen((string("weight/") + to_string(idx) + string(".bin")).c_str(),"wb");
        FILE* rand_file = fopen((string("random/") + to_string(idx) + string(".bin")).c_str(),"wb");
        FILE* ctxt_file = fopen((string("product/") + to_string(idx) + string(".bin")).c_str(),"wb");
        mpz_out_raw(ptxt_file, weight[idx]);
        mpz_out_raw(rand_file, random[idx]);
        mpz_out_raw(ctxt_file, product[idx]);
        mpz_clear(product[idx]);
        mpz_clear(ciphertext[idx]);
        mpz_clear(random[idx]);
        mpz_clear(weight[idx]);
        fclose(ptxt_file);
        fclose(rand_file);
        fclose(ctxt_file);
    }
    paillier_public_clear(pub_key);

    return 0;
}