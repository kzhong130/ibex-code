#include <assert.h>

#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "seal/seal.h"

using namespace std;
using namespace seal;
using namespace chrono;

class HE {
   private:
    int log_poly_degree = 15;
    size_t poly_modulus_degree;
    int plain_modulus = (1 << 27);

    Encryptor* encryptor;
    Evaluator* evaluator;
    Decryptor* decryptor;

    inline vector<int> modulus_bits() {
        switch (log_poly_degree) {
            case 15: {
                return {50, 24, 23};
            }
            case 16: {
                return {51, 24, 24};
            }
            case 17: {
                return {52, 24, 24};
            }
            default:
                assert(0);
        }
    }

    void encodeShare(int id, Plaintext& p) {
        assert(id <= poly_modulus_degree);
        string poly = "1x^";
        poly.append(to_string(id));
        p = Plaintext(poly);
    }

   public:
    HE() {}

    HE(int log_group, int plain_modulus_ = (1 << 27)) {
        log_poly_degree = log_group + 1;
        plain_modulus = plain_modulus_;
        EncryptionParameters parms(scheme_type::bfv);
        poly_modulus_degree = (1 << log_poly_degree);
        parms.set_poly_modulus_degree(poly_modulus_degree);
        parms.set_coeff_modulus(
            CoeffModulus::Create(poly_modulus_degree, modulus_bits()));
        parms.set_plain_modulus(plain_modulus);
        SEALContext context_(parms, true, sec_level_type::none);

        KeyGenerator keygen(context_);
        SecretKey secret_key = keygen.secret_key();
        PublicKey public_key;
        keygen.create_public_key(public_key);

        encryptor = new Encryptor(context_, public_key);
        evaluator = new Evaluator(context_);
        decryptor = new Decryptor(context_, secret_key);
    }

    void encShare(int id, Ciphertext& c) {
        Plaintext p;
        encodeShare(id, p);
        encryptor->encrypt(p, c);
    }

    void dec(Ciphertext& c, Plaintext& p) { decryptor->decrypt(c, p); }

    void shift(Ciphertext& cs, int s, Ciphertext& dst) {
        Plaintext p;
        encodeShare(s, p);
        // Ciphertext c;
        // encryptor->encrypt(p, c);
        evaluator->multiply_plain(cs, p, dst);
    }

    void agg(Ciphertext& c1, Ciphertext& c2, Ciphertext& c12) {
        evaluator->add(c1, c2, c12);
    }

    int aggLimit() {
        Ciphertext c;
        encShare(2, c);
        shift(c, 2, c);
        int times_log = 0;
        while (true) {
            agg(c, c, c);
            if (decryptor->invariant_noise_budget(c) <= 0) break;
            times_log++;
        }
        return times_log;
    }

    string serializeCiphertext(Ciphertext& c) {
        stringstream data_stream;
        c.save(data_stream);
        return data_stream.str();
    }
};