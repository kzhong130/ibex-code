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

    system_clock::time_point start, end;
    start = system_clock::now();
    LWE lwe(pStr, wStr, nbits, pd_str, LOGP, false);
    end = system_clock::now();
    cout << "KEYGEN: "
         << duration_cast<std::chrono::duration<double>>(end - start).count()
         << endl;

    // ciphertext addition test
    int polyDegree = (1 << nbits);
    Plaintext p1, p2, p12;
    int id1, id2;
    p1.m = initRandBitVec(polyDegree, id1);
    p2.m = initRandBitVec(polyDegree, id2);
    int id12 = (id1 + id2) % polyDegree;
    Ciphertext c1 = lwe.enc(p1);
    Ciphertext c2 = lwe.enc(p2);
    Ciphertext cmul = lwe.mul(c1, c2);
    Plaintext pmul = lwe.dec(cmul);
    // check whether pmul is correct
    for (int i = 0; i < polyDegree; i++) {
        if (!((i != id12 && mpz_cmp_si(pmul.m[i], 0) == 0) ||
              (i == id12 && mpz_cmp_si(pmul.m[i], 1) == 0))) {
            cout << id1 << " " << id2 << " " << id12 << " " << i << endl;
            mpz_out_str(stdout, 10, pmul.m[i]);
            cout << endl;
        }
        assert((i != id12 && mpz_cmp_si(pmul.m[i], 0) == 0) ||
               (i == id12 && mpz_cmp_si(pmul.m[i], 1) == 0));
    }
    int counter = 0;
    while (true) {
        lwe.mul2Exp(cmul, 1);
        lwe.r.ringMul2ExpPlainMod(pmul.m, 1);
        Plaintext res = lwe.dec(cmul);
        if (!pmul.equal(res)) {
            break;
        } else {
            counter++;
            cout << counter << " add test passed\n";
        }
    }
    cout << "test finish, final addition times: 2^{" << counter << "}" << endl;

    return 0;
}
