#include "lwe.cpp"

vector<mpz_t> initVec(vector<int>& v) {
    vector<mpz_t> res = vector<mpz_t>(v.size());
    for (int i = 0; i < v.size(); i++) {
        mpz_init(res[i]);
        mpz_set_si(res[i], v[i]);
    }
    return res;
}

vector<mpz_t> initRandBitVec(int degree, int& idx) {
    vector<mpz_t> res = vector<mpz_t>(degree);
    for (int i = 0; i < degree; i++) {
        mpz_init(res[i]);
    }
    idx = rand() % (degree / 2);
    mpz_set_si(res[idx], 1);
    return res;
}

vector<mpz_t> initExpandBitVec(int degree, int& idx) {
    vector<mpz_t> res = vector<mpz_t>(degree);
    for (int i = 0; i < degree; i++) {
        mpz_init(res[i]);
    }
    idx = rand() % degree;
    mpz_set_si(res[idx], 1);
    return res;
}

int main(int argc, char* argv[]) {
    int nbits = 16;
    string pd_str = "134217728";
    int LOGP = 109;
    string pStr = "558127740940706268294329795608577";
    string wStr = "3";
    system_clock::time_point start, end;
    start = system_clock::now();
    bool ks = false;

    int opt;
    while ((opt = getopt(argc, argv, "b:d:l:p:w:")) != -1) {
        if (opt == 'b') {
            nbits = atoi(optarg);
        } else if (opt == 'd') {
            pd_str = string(optarg);
        } else if (opt == 'l') {
            LOGP = atoi(optarg);
        } else if (opt == 'p') {
            pStr = string(optarg);
        } else if (opt == 'w') {
            wStr = string(optarg);
        }
    }

    LWE lwe(pStr, wStr, nbits, pd_str, LOGP, ks);

    end = system_clock::now();
    cout << "KEYGEN: "
         << duration_cast<std::chrono::duration<double>>(end - start).count()
         << endl;

    int encTimes = 1;
    // enc and dec test
    for (int i = 0; i < encTimes; i++) {
        int polyDegree = (1 << nbits);
        Plaintext p;
        p.m = lwe.r.randPlainMod();
        system_clock::time_point start, end;
        start = system_clock::now();
        Ciphertext c = lwe.enc(p);
        end = system_clock::now();
        cout
            << i << " ENC TIME: "
            << duration_cast<std::chrono::duration<double>>(end - start).count()
            << endl;

        Plaintext res = lwe.dec(c);
        assert(res.equal(p));
        cout << i << " dec passed\n";
    }

    // ciphertext addition test
    int addTimes = 1;
    for (int i = 0; i < addTimes; i++) {
        int polyDegree = (1 << nbits);
        Plaintext p1, p2, p12;
        p1.m = lwe.r.randPlainMod();
        p2.m = lwe.r.randPlainMod();
        p12.m = lwe.r.ringAddPlainMod(p1.m, p2.m);
        Ciphertext c1 = lwe.enc(p1);
        Ciphertext c2 = lwe.enc(p2);
        system_clock::time_point start, end;
        start = system_clock::now();
        Ciphertext c12 = lwe.add(c1, c2);
        end = system_clock::now();
        cout
            << i << " ADD TIME: "
            << duration_cast<std::chrono::duration<double>>(end - start).count()
            << endl;

        Plaintext res = lwe.dec(c12);

        assert(res.equal(p12));
        cout << i << " add test passed\n";
    }

    // ciphertext mul test
    int mulTimes = 5;
    for (int t = 0; t < mulTimes; t++) {
        int polyDegree = (1 << nbits);
        Plaintext p1, p2, p12;
        int id1, id2;
        p1.m = initRandBitVec(polyDegree, id1);
        p2.m = initRandBitVec(polyDegree, id2);
        int id12 = (id1 + id2) % polyDegree;
        Ciphertext c1 = lwe.enc(p1);
        Ciphertext c2 = lwe.enc(p2);
        system_clock::time_point start, end;
        start = system_clock::now();
        Ciphertext cmul = lwe.mul(c1, c2);
        end = system_clock::now();
        Plaintext res = lwe.dec(cmul);
        cout
            << t << " MUL TIME: "
            << duration_cast<std::chrono::duration<double>>(end - start).count()
            << endl;

        for (int i = 0; i < polyDegree; i++) {
            if (!((i != id12 && mpz_cmp_si(res.m[i], 0) == 0) ||
                  (i == id12 && mpz_cmp_si(res.m[i], 1) == 0))) {
                cout << id1 << " " << id2 << " " << id12 << " " << i << endl;
                mpz_out_str(stdout, 10, res.m[i]);
                cout << endl;
            }
            assert((i != id12 && mpz_cmp_si(res.m[i], 0) == 0) ||
                   (i == id12 && mpz_cmp_si(res.m[i], 1) == 0));
        }
        cout << t << " mul test passed\n";
    }

    // if ks is not true, skip remaining tests
    if (ks == false) return 0;

    int ksTimes = 5;
    for (int i = 0; i < ksTimes; i++) {
        int idx = rand() % nbits;
        int polyDegree = (1 << nbits);
        Plaintext p;
        int id = 0;
        p.m = initRandBitVec(polyDegree, id);
        Ciphertext c = lwe.enc(p);
        Ciphertext sub_cipher = lwe.substitute(c, idx);
        Plaintext sub_p = lwe.dec(sub_cipher);
        cout << i << " key switch passed\n";
    }

    int expandTimes = 5;
    for (int t = 0; t < expandTimes; t++) {
        int idx = rand() % nbits;
        int polyDegree = (1 << nbits);
        Plaintext p;
        int id = 0;
        p.m = initExpandBitVec(polyDegree, id);
        cout << "original id: " << id << endl;
        Ciphertext c = lwe.enc(p);
        system_clock::time_point start, end;
        start = system_clock::now();
        vector<Ciphertext> res = lwe.expand(c);
        end = system_clock::now();
        cout
            << t << " EXPAND TIME: "
            << duration_cast<std::chrono::duration<double>>(end - start).count()
            << endl;
        Plaintext zero, expand_res;
        zero.m = vector<mpz_t>(1 << nbits);
        expand_res.m = vector<mpz_t>(1 << nbits);
        for (int i = 0; i < (1 << nbits); i++) {
            mpz_init(zero.m[i]);
            mpz_init(expand_res.m[i]);
        }
        mpz_set_si(expand_res.m[0], (1 << nbits));
        Plaintext dec_res = lwe.dec(res[id]);
        assert(expand_res.equal(dec_res));
        cout << "passed test for " << id << " to see if it's correct\n";
        // randomly sample three random `idx` and decrypt the ciphertext to
        // see if they decrypt to be all zeros
        for (int i = 0; i < 3; i++) {
            int test_idx = rand() % (1 << nbits);
            while (test_idx == id) {
                test_idx = rand() % (1 << nbits);
            }
            cout << "testing " << test_idx << " see if it's all zeros\n";
            Plaintext dec_zero = lwe.dec(res[test_idx]);
            assert(zero.equal(dec_zero));
        }
        cout << t << " expand test passed\n";
    }

    return 0;
}
