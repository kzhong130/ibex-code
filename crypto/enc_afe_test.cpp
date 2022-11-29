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
    int nbits = 15;
    // a 99-100 bit integer close to 2**100
    string pd_str = "1267650600228229401496703205371";
    int LOGP = 150;
    string pStr = "950262814350677829299610740348228965123489793";
    string wStr = "5";
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
    int addTimes = 100;
    Plaintext p1;
    p1.m = lwe.r.randPlainMod();
    Ciphertext c1 = lwe.enc(p1);
    for (int i = 0; i < addTimes; i++) {
        Plaintext p2;
        p2.m = lwe.r.randPlainMod();
        p1.m = lwe.r.ringAddPlainMod(p1.m, p2.m);
        Ciphertext c2 = lwe.enc(p2);
        system_clock::time_point start, end;
        start = system_clock::now();
        c1 = lwe.add(c1, c2);
        end = system_clock::now();
        cout
            << i << " ADD TIME: "
            << duration_cast<std::chrono::duration<double>>(end - start).count()
            << endl;

        Plaintext res = lwe.dec(c1);

        assert(res.equal(p1));
        cout << i << " add test passed\n";
    }

    return 0;
}
