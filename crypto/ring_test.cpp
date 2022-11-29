#include "ring.cpp"

#include "getopt.h"
#include "stdlib.h"

void initVec(vector<mpz_t> &v, mpz_t &p, gmp_randstate_t &state) {
    for (int i = 0; i < v.size(); i++) {
        mpz_init(v[i]);
        mpz_urandomm(v[i], state, p);
    }
}

void testRingMul(bool bench, int nbits) {
    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(0));
    mpz_t p;
    string p_str =
        "7821965511409082574747680203548583888845963695369756268334572559420447"
        "7452289";
    string w_str = "3";
    string pd_str = "33554432";
    mpz_init(p);
    mpz_set_str(p, "300424569129657234489620267994584186881", 10);
    Ring r(string("300424569129657234489620267994584186881"), string("7"),
           nbits, pd_str);

    for (int i = 0; i < 10; i++) {
        vector<mpz_t> a = vector<mpz_t>((1 << nbits));
        vector<mpz_t> b = vector<mpz_t>((1 << nbits));
        initVec(a, p, state);
        initVec(b, p, state);

        system_clock::time_point start, end;
        start = system_clock::now();
        vector<mpz_t> res1 = r.ringMul(a, b);
        end = system_clock::now();

        cout
            << "TIME: "
            << duration_cast<std::chrono::duration<double>>(end - start).count()
            << endl;
        if (!bench) {
            vector<mpz_t> res2 = r.ringMulSlow(a, b);

            for (int i = 0; i < res1.size(); i++) {
                if (mpz_cmp(res1[i], res2[i]) != 0) {
                    cout << i << " fast: ";
                    mpz_out_str(stdout, 10, res1[i]);
                    cout << endl;
                    cout << i << " slow: ";
                    mpz_out_str(stdout, 10, res2[i]);
                    cout << endl;
                    assert(0);
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int opt;
    bool bench = false;
    int nbits = 15;
    while ((opt = getopt(argc, argv, "bd:")) != -1) {
        if (opt == 'b') {
            bench = true;
        } else if (opt == 'd') {
            nbits = atoi(optarg);
        }
    }

    testRingMul(bench, nbits);
    return 0;
}