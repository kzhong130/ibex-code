#include <iostream>
#include <chrono>
#include "tools.c"
#include "paillier.c"

using namespace std;
using namespace chrono;

int main() {
    system_clock::time_point starttime, endtime;

    int vector_length = 1024;
    mpz_t feature[vector_length], random[vector_length], weight[vector_length], product[vector_length],
            ptxt_result_1[vector_length], ptxt_result_2[vector_length];

    // Read private key
    paillier_private_key* priv_key = new paillier_private_key();
    FILE* priv_key_file = fopen("private.key","rb");
	paillier_private_init(priv_key);
	paillier_private_in_str(priv_key, priv_key_file);
    fclose(priv_key_file);

    // Init vectors
	for (int idx = 0; idx < vector_length; idx++) {
        FILE* feature_file = fopen((string("feature/") + to_string(idx) + string(".bin")).c_str(),"rb");
        mpz_init(feature[idx]);
        mpz_inp_raw(feature[idx], feature_file);
        fclose(feature_file);
        FILE* rand_file = fopen((string("random/") + to_string(idx) + string(".bin")).c_str(),"rb");
        mpz_init(random[idx]);
        mpz_inp_raw(random[idx], rand_file);
        fclose(rand_file);
        FILE* weight_file = fopen((string("weight/") + to_string(idx) + string(".bin")).c_str(),"rb");
        mpz_init(weight[idx]);
        mpz_inp_raw(weight[idx], weight_file);
        fclose(weight_file);
        FILE* product_file = fopen((string("product/") + to_string(idx) + string(".bin")).c_str(),"rb");
        mpz_init(product[idx]);
        mpz_inp_raw(product[idx], product_file);
        fclose(product_file);
    }

    // Decrypt
    starttime = system_clock::now();
    for (int idx = 0; idx < vector_length; idx++) {
        mpz_init(ptxt_result_1[idx]);
        paillier_decrypt(ptxt_result_1[idx], product[idx], priv_key);
    }
    endtime = system_clock::now();
    cout << "Decryption time: " << duration_cast<duration<double>>(endtime - starttime).count() << endl;

    for (int idx = 0; idx < vector_length; idx++) {
        mpz_init(ptxt_result_2[idx]);
        mpz_mul(ptxt_result_2[idx], weight[idx], feature[idx]);
        mpz_add(ptxt_result_2[idx], ptxt_result_2[idx], random[idx]);
        mpz_sub(ptxt_result_1[idx], ptxt_result_1[idx], ptxt_result_2[idx]);
        if (mpz_sgn(ptxt_result_1[idx]) != 0) {
            cout << "Error in inference!" << endl;
            exit(-1);
        }
        mpz_clear(feature[idx]);
        mpz_clear(random[idx]);
        mpz_clear(weight[idx]);
        mpz_clear(product[idx]);
        mpz_clear(ptxt_result_1[idx]);
        mpz_clear(ptxt_result_2[idx]);
    }
    paillier_private_clear(priv_key);

    return 0;
}