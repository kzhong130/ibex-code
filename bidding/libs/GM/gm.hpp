#include <assert.h>
#include <gmp.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;
using namespace chrono;

class GM {
   private:
    gmp_randstate_t randState;
    mpz_t N;
    mpz_t pk_a;
    mpz_t p;
    mpz_t q;

    void randNum(mpz_t& r) {
        mpz_init(r);
        mpz_urandomm(r, this->randState, this->N);
    }

   public:
    GM() {}

    GM(string pk_file, string sk_file) {
        gmp_randinit_default(this->randState);
        gmp_randseed_ui(this->randState, time(0));
        mpz_init(this->p);
        mpz_init(this->q);
        mpz_init(this->pk_a);
        mpz_init(this->N);
        ifstream pf(pk_file);
        stringstream pkss;
        pkss << pf.rdbuf();
        string pk_str = pkss.str();
        char* pk_ptr = (char*)pk_str.c_str();
        int N_len = deserialize_mpz(pk_ptr, this->N);
        int pk_a_len = deserialize_mpz(pk_ptr + N_len, this->pk_a);
        pf.close();

        ifstream sf(sk_file);
        stringstream skss;
        skss << sf.rdbuf();
        string sk_str = skss.str();
        char* sk_ptr = (char*)sk_str.c_str();
        int p_len = deserialize_mpz(sk_ptr, this->p);
        int q_len = deserialize_mpz(sk_ptr + p_len, this->q);
        sf.close();
        cout << "bit size of read p: " << mpz_sizeinbase(this->p, 2) << endl;
        cout << "bit size of read q: " << mpz_sizeinbase(this->q, 2) << endl;
        cout << "bit size of read N: " << mpz_sizeinbase(this->N, 2) << endl;
    }

    GM(int sk_bit) {
        gmp_randinit_default(this->randState);
        gmp_randseed_ui(this->randState, time(0));
        mpz_init(this->p);
        mpz_init(this->q);
        mpz_init(this->pk_a);
        mpz_init(this->N);

        mpz_rrandomb(this->p, this->randState, sk_bit);
        mpz_rrandomb(this->q, this->randState, sk_bit);

        if (mpz_probab_prime_p(this->p, 15) != 2) {
            mpz_nextprime(this->p, this->p);
        }

        if (mpz_probab_prime_p(this->q, 15) != 2) {
            mpz_nextprime(this->q, this->q);
        }

        mpz_mul(this->N, this->p, this->q);

        cout << "bit size of generated N: " << mpz_sizeinbase(this->N, 2)
             << endl;

        while (true) {
            mpz_urandomm(this->pk_a, this->randState, this->N);
            if ((mpz_jacobi(this->pk_a, this->p) == -1) &&
                (mpz_jacobi(this->pk_a, this->q) == -1))
                break;
        }

        cout << "bit size of generated pk_a: " << mpz_sizeinbase(this->pk_a, 2)
             << endl;
    }

    string serialize_mpz(mpz_t val) {
        int outlen = 1024;
        char tmp[outlen];
        FILE* fo = fmemopen(tmp, outlen, "w");
        size_t binlen = mpz_out_raw(fo, val);
        assert(binlen < outlen);
        fclose(fo);
        string input = string((const char*)tmp, binlen);

        char len[sizeof(uint32_t)];
        uint32_t str_len = input.size();
        memcpy(len, &str_len, sizeof(uint32_t));
        string prefix = string(len, sizeof(uint32_t));
        prefix.append(input);
        assert(prefix.size() == input.size() + sizeof(uint32_t));
        return prefix;
    }

    // return the total length that the ptr has been read
    int deserialize_mpz(char* ptr, mpz_t& val) {
        uint32_t size = *((uint32_t*)ptr);
        FILE* f = fmemopen(ptr + sizeof(uint32_t), size, "r");
        size_t read_len = mpz_inp_raw(val, f);
        fclose(f);
        assert(read_len == size);
        return size + sizeof(uint32_t);
    }

    void output_keys(string pk_file, string sk_file) {
        ofstream pf(pk_file);
        ofstream sf(sk_file);
        string N_str = serialize_mpz(this->N);
        string pk_a_str = serialize_mpz(this->pk_a);
        string p_str = serialize_mpz(this->p);
        string q_str = serialize_mpz(this->q);
        pf << N_str << pk_a_str;
        pf.close();
        sf << p_str << q_str;
        sf.close();
    }

    int enc(int bit, mpz_t& c) {
        if (!(bit == 0 || bit == 1)) {
            return -1;
        }
        mpz_init(c);
        mpz_t r;
        randNum(r);
        mpz_mul(c, r, r);
        if (bit == 1) {
            mpz_mul(c, c, this->pk_a);
        }
        mpz_mod(c, c, this->N);
        return 0;
    }

    // -1 indicates decryption failed
    int dec(mpz_t& c) {
        int v = mpz_legendre(c, this->p);
        if (v == 1) {
            return 0;
        } else if (v == -1) {
            return 1;
        } else {
            return -1;
        }
    }

    void cipherAdd(mpz_t& r, mpz_t& c1, mpz_t& c2) {
        mpz_init(r);
        mpz_mul(r, c1, c2);
        mpz_mod(r, r, this->N);
    }
};
