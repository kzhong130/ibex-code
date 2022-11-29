#include <assert.h>
#include <gmp.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <chrono>
#include <iostream>
#include <vector>

using namespace std;
using namespace chrono;

void printBigNum(vector<mpz_t>& v, string custom = "") {
    for (int i = 0; i < v.size(); i++) {
        cout << i << " " << custom << " ";
        mpz_out_str(stdout, 10, v[i]);
        cout << endl;
    }
}

void clearRing(vector<mpz_t>& v) {
    for (int i = 0; i < v.size(); i++) {
        mpz_clear(v[i]);
    }
}

vector<mpz_t> copyBigNum(vector<mpz_t>& v) {
    vector<mpz_t> res(v.size());
    for (int i = 0; i < v.size(); i++) {
        mpz_init(res[i]);
        mpz_set(res[i], v[i]);
    }
    return res;
}

class Ring {
   private:
    mpz_t p;     // prime ringModulus
    mpz_t w;     // root of unity, used for computing NTT
    int nbits;   // bits of polynomial degree
    int degree;  // polynomial degree
    vector<mpz_t> omega;
    vector<mpz_t> omegaInv;
    mpz_t invN;
    gmp_randstate_t randState;
    mpz_t N;         // N = degree in mpz_t type
    mpz_t twoN;      // twoN = 2 * degree in mpz_t type
    mpz_t plainMod;  // plaintext modulus
    mpz_t halfMthP;

    mpz_t degree_inverse;  // N^-1

    vector<mpz_t> transform(vector<mpz_t>& inputs, vector<mpz_t>& omega) {
        int n = this->degree * 2;
        vector<mpz_t> a = vector<mpz_t>(inputs.size());
        for (int i = 0; i < a.size(); i++) {
            mpz_init(a[i]);
            mpz_set(a[i], inputs[i]);
        }

        for (int i = 0, j = 0; i < n; i++) {
            if (i > j) {
                swap(a[i], a[j]);
            }
            for (int l = n >> 1;; l >>= 1) {
                j ^= l;
                if (j >= l) {
                    break;
                }
            }
        }
        for (int l = 2; l <= n; l <<= 1) {
            int m = (l >> 1);

            for (int st = (0); st < n; st += l) {
                // b := a[st:]
                mpz_t w;
                mpz_init(w);
                mpz_set_si(w, 1);

                for (int i = (0); i < m; i++) {
                    // d := b[m+i] * w % r.p
                    mpz_t d;
                    mpz_init(d);
                    mpz_mul(d, a[st + m + i], w);
                    ringMod(d);

                    mpz_sub(a[st + m + i], a[st + i], d);
                    ringMod(a[st + m + i]);

                    mpz_add(a[st + m + i], a[st + m + i], p);
                    ringMod(a[st + m + i]);

                    // b[m+i].Add(b[m+i], r.p)
                    mpz_add(a[st + i], a[st + i], d);
                    ringMod(a[st + i]);

                    // w = w * wn % r.p
                    mpz_mul(w, w, omega[l]);
                    ringMod(w);
                    mpz_clear(d);
                }
                mpz_clear(w);
            }
        }
        return a;
    }

    vector<mpz_t> dft(vector<mpz_t>& a) { return transform(a, omega); }

    vector<mpz_t> idft(vector<mpz_t>& a) {
        vector<mpz_t> res = transform(a, omegaInv);
        for (int i = 0; i < res.size(); i++) {
            // res[i] = v * r.invN % r.p
            mpz_mul(res[i], res[i], this->invN);
            ringMod(res[i]);
        }
        return res;
    }

   public:
    Ring() {}
    Ring(string pStr, string wStr, int nbits, string pd) {
        // Set Plaintext degree
        mpz_init(this->plainMod);
        mpz_set_str(this->plainMod, pd.c_str(), 10);

        // init p
        mpz_init(this->p);
        mpz_set_str(this->p, pStr.c_str(), 10);

        // init w
        mpz_init(this->w);
        mpz_set_str(this->w, wStr.c_str(), 10);

        // set nbits
        this->nbits = nbits;

        // set degree
        this->degree = (1 << nbits);

        // set N and twoN
        mpz_init(this->twoN);
        mpz_set_si(this->twoN, 2 * this->degree);
        mpz_init(this->N);
        mpz_set_si(this->N, this->degree);

        int n = (1 << nbits) * 2;

        // init omega and omegaInv
        this->omega = vector<mpz_t>(n + 1);
        this->omegaInv = vector<mpz_t>(n + 1);

        mpz_t invw;
        mpz_init(invw);
        mpz_invert(invw, this->w, this->p);
        // mpz_set_str(invw, invwStr.c_str(), 10);

        mpz_init(this->invN);
        mpz_t n_big;
        mpz_init(n_big);
        mpz_set_si(n_big, n);
        mpz_t exp;
        mpz_init(exp);
        mpz_set_si(exp, 2);
        mpz_sub(exp, this->p, exp);

        // n^{p-2} ringMod p
        mpz_powm(this->invN, n_big, exp, this->p);

        for (int i = 1; i <= n; i <<= 1) {
            mpz_t exp;
            mpz_init(exp);
            mpz_sub_ui(exp, p, 1);
            mpz_div_ui(exp, exp, i);

            mpz_init(omega[i]);
            mpz_init(omegaInv[i]);
            mpz_powm(omega[i], this->w, exp, this->p);
            mpz_powm(omegaInv[i], invw, exp, this->p);
        }
        gmp_randinit_default(this->randState);
        gmp_randseed_ui(this->randState, time(0));
        srand(time(0));

        // init halfMthP
        mpz_init(halfMthP);
        mpz_div_ui(halfMthP, this->p, 2);

        // init m_inverse
        mpz_init(this->degree_inverse);
        mpz_invert(this->degree_inverse, this->N, this->p);
    }

    void ringMod(mpz_t& v) {
        mpz_mod(v, v, this->p);
        if (mpz_sgn(v) < 0) {
            assert(0);
            mpz_add(v, v, this->p);
        }
    }

    // length of return result is this->degree
    vector<mpz_t> ringMulSlow(vector<mpz_t>& a, vector<mpz_t>& b) {
        assert((a.size() == this->degree && b.size() == this->degree));
        vector<mpz_t> conv = vector<mpz_t>(this->degree * 2);
        for (int i = 0; i < conv.size(); i++) {
            mpz_init(conv[i]);
        }

        for (int i = 0; i < this->degree; i++) {
            for (int j = 0; j < this->degree; j++) {
                mpz_addmul(conv[i + j], a[i], b[j]);
                ringMod(conv[i + j]);
            }
        }

        for (int i = 0; i < this->degree - 1; i++) {
            mpz_sub(conv[i], conv[i], conv[i + this->degree]);
            ringMod(conv[i]);
        }
        vector<mpz_t> res = vector<mpz_t>(this->degree);
        for (int i = 0; i < this->degree; i++) {
            mpz_init(res[i]);
            mpz_set(res[i], conv[i]);
        }
        return res;
    }

    vector<mpz_t> ringAdd(vector<mpz_t>& a, vector<mpz_t>& b) {
        assert(a.size() == b.size());
        assert(a.size() == degree);
        vector<mpz_t> res = vector<mpz_t>(degree);
        for (int i = 0; i < degree; i++) {
            mpz_init(res[i]);
            mpz_add(res[i], a[i], b[i]);
            ringMod(res[i]);
        }
        return res;
    }

    vector<mpz_t> ringAddPlainMod(vector<mpz_t>& a, vector<mpz_t>& b) {
        assert(a.size() == b.size());
        assert(a.size() == degree);
        vector<mpz_t> res = vector<mpz_t>(degree);
        for (int i = 0; i < degree; i++) {
            mpz_init(res[i]);
            mpz_add(res[i], a[i], b[i]);
            mpz_mod(res[i], res[i], plainMod);
        }
        return res;
    }

    vector<mpz_t> ringSub(vector<mpz_t>& a, vector<mpz_t>& b) {
        assert(a.size() == b.size());
        assert(a.size() == degree);
        vector<mpz_t> res = vector<mpz_t>(degree);
        for (int i = 0; i < degree; i++) {
            mpz_init(res[i]);
            mpz_sub(res[i], a[i], b[i]);
            ringMod(res[i]);
        }
        return res;
    }

    vector<mpz_t> ringMul(vector<mpz_t>& inputA, vector<mpz_t>& inputB) {
        assert(inputA.size() == this->degree && inputB.size() == this->degree);
        vector<mpz_t> a = vector<mpz_t>(this->degree * 2);
        vector<mpz_t> b = vector<mpz_t>(this->degree * 2);

        for (int i = 0; i < this->degree * 2; i++) {
            mpz_init(a[i]);
            mpz_init(b[i]);
            if (i < this->degree) {
                mpz_set(a[i], inputA[i]);
                mpz_set(b[i], inputB[i]);
            }
        }

        vector<mpz_t> ntta = dft(a);
        vector<mpz_t> nttb = dft(b);

        for (int i = 0; i < a.size(); i++) {
            mpz_mul(ntta[i], ntta[i], nttb[i]);
            ringMod(ntta[i]);
        }

        vector<mpz_t> intt = idft(ntta);

        for (int i = 0; i < this->degree - 1; i++) {
            mpz_sub(intt[i], intt[i], intt[i + this->degree]);
            ringMod(intt[i]);
        }
        vector<mpz_t> res = vector<mpz_t>(this->degree);
        for (int i = 0; i < this->degree; i++) {
            mpz_init(res[i]);
            mpz_set(res[i], intt[i]);
        }

        clearRing(a);
        clearRing(b);
        clearRing(ntta);
        clearRing(nttb);
        clearRing(intt);
        return res;
    }

    // generate a vector of random elements from 0 to p
    vector<mpz_t> randRing() {
        vector<mpz_t> res = vector<mpz_t>(this->degree);
        for (int i = 0; i < this->degree; i++) {
            mpz_init(res[i]);
            mpz_urandomm(res[i], this->randState, this->p);
        }
        return res;
    }

    // generate a vector of random elements from 0 to plainMod
    vector<mpz_t> randPlainMod() {
        vector<mpz_t> res = vector<mpz_t>(this->degree);
        for (int i = 0; i < this->degree; i++) {
            mpz_init(res[i]);
            mpz_urandomm(res[i], this->randState, this->plainMod);
        }
        return res;
    }

    // generate a vector of random elements from -N to N
    vector<mpz_t> randBinom() {
        vector<mpz_t> res = vector<mpz_t>(this->degree);
        for (int i = 0; i < this->degree; i++) {
            mpz_init(res[i]);
            mpz_urandomm(res[i], this->randState, this->twoN);
            mpz_sub(res[i], res[i], this->N);
            ringMod(res[i]);
        }
        return res;
    }

    // generate a vector of random elements from -1 to 1
    vector<mpz_t> randBits() {
        vector<mpz_t> res = vector<mpz_t>(this->degree);
        for (int i = 0; i < this->degree; i++) {
            mpz_init(res[i]);
            int r = rand() % (2 * 2 - 1) - 1;
            assert(r >= -1 && r <= 1);
            mpz_set_si(res[i], r);
        }
        return res;
    }

    void ringMulPlainMod(vector<mpz_t>& input) {
        for (int i = 0; i < input.size(); i++) {
            mpz_mul(input[i], input[i], plainMod);
            ringMod(input[i]);
        }
    }

    void ringMul2Exp(vector<mpz_t>& input, int exp) {
        for (int i = 0; i < input.size(); i++) {
            mpz_mul_2exp(input[i], input[i], exp);
            ringMod(input[i]);
        }
    }

    void ringMul2ExpPlainMod(vector<mpz_t>& input, int exp) {
        for (int i = 0; i < input.size(); i++) {
            mpz_mul_2exp(input[i], input[i], exp);
            mpz_mod(input[i], input[i], plainMod);
        }
    }

    void ringMulDegreeInv(vector<mpz_t>& input) {
        for (int i = 0; i < input.size(); i++) {
            mpz_mul(input[i], input[i], degree_inverse);
            ringMod(input[i]);
        }
    }

    // mod each element with plaintext degree
    void ModPd(vector<mpz_t>& input) {
        for (int i = 0; i < input.size(); i++) {
            if (mpz_cmp(input[i], halfMthP) > 0) {
                mpz_sub(input[i], input[i], p);
            }
            mpz_mod(input[i], input[i], plainMod);
        }
    }

    int getplainDegree() { return degree; }

    vector<mpz_t> subsitute(vector<mpz_t>& c, int pow) {
        vector<mpz_t> res(c.size());
        for (int i = 0; i < c.size(); i++) {
            mpz_init(res[i]);
        }
        for (int i = 0; i < c.size(); i++) {
            int idx = (i * pow) % degree;
            if (((i * pow) / degree) % 2 == 0) {
                mpz_add(res[idx], res[idx], c[i]);
            } else {
                mpz_sub(res[idx], res[idx], c[i]);
            }
            ringMod(res[idx]);
        }
        return res;
    }

    // `shift_idx` must be non-negative number
    // `neg` indicates whether `shift_idx` used to be a negative number,
    // and it's transformed from a negative number to positive number
    vector<mpz_t> ringShift(vector<mpz_t>& c, int shift_idx, bool neg) {
        assert(shift_idx >= 0);
        vector<mpz_t> res(c.size());
        for (int i = 0; i < c.size(); i++) {
            mpz_init(res[i]);
        }
        int flag = 0;
        if (neg) {
            flag = 1;
        }
        for (int i = 0; i < c.size(); i++) {
            int idx = (i + shift_idx) % degree;
            if ((((i + shift_idx) / degree) + flag) % 2 == 0) {
                mpz_add(res[idx], res[idx], c[i]);
            } else {
                mpz_sub(res[idx], res[idx], c[i]);
            }
            ringMod(res[idx]);
        }
        return res;
    }
};