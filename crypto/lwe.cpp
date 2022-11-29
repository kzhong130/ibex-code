#include "ring.cpp"
#define KEY_SWITCHING_NUM 20
// size of ciphertext num
#define NUM_SIZE 200
// LOGP needs to be mannully modified when you change logP inputs
// #define LOGP 128
// #define LOGP 550

class Ciphertext {
   public:
    int num;
    // u = c[1], v = c[0]
    vector<vector<mpz_t>> c;

    // init a ciphertex of size n
    Ciphertext(int n) {
        num = n;
        c = vector<vector<mpz_t>>(num);
    }

    // by default init a ciphertext of two
    Ciphertext() {
        num = 2;
        c = vector<vector<mpz_t>>(num);
    }

    Ciphertext copy() {
        Ciphertext res(this->num);
        for (int i = 0; i < res.num; i++) {
            res.c[i] = vector<mpz_t>(this->c[i].size());
            for (int j = 0; j < this->c[i].size(); j++) {
                mpz_init(res.c[i][j]);
                mpz_set(res.c[i][j], this->c[i][j]);
            }
        }
        return res;
    }

    void free() {
        for (int i = 0; i < this->num; i++) {
            clearRing(this->c[i]);
        }
    }

    string serialize() {
        string res;
        for (int i = 0; i < num; i++) {
            for (int j = 0; j < this->c[i].size(); j++) {
                char tmp[NUM_SIZE];
                FILE* fo = fmemopen(tmp, NUM_SIZE, "w");
                size_t binlen = mpz_out_raw(fo, this->c[i][j]);
                assert(binlen < NUM_SIZE);
                res.append(string((const char*)tmp, binlen));
                fclose(fo);
            }
        }
        return res;
    }
};

class Plaintext {
   public:
    vector<mpz_t> m;

    bool equal(Plaintext& p) {
        if (p.m.size() != m.size()) {
            cout << "mismatched size " << p.m.size() << " " << m.size() << endl;
            return false;
        }
        for (int i = 0; i < m.size(); i++) {
            if (mpz_cmp(p.m[i], m[i]) != 0) {
                return false;
            }
        }
        return true;
    }

    void free() { clearRing(m); }

    Plaintext copy() {
        Plaintext res;
        res.m = vector<mpz_t>(this->m.size());
        for (int i = 0; i < this->m.size(); i++) {
            mpz_init(res.m[i]);
            mpz_set(res.m[i], this->m[i]);
        }
        return res;
    }
};

class LWE {
   public:
    Ring r;

   private:
    vector<mpz_t> pk[2];
    vector<mpz_t> sk;
    int nbits;
    int logP;  // log(pStr)
    vector<mpz_t> gadget;
    vector<mpz_t> substitution_keys[KEY_SWITCHING_NUM];
    // sk[0] = u, sk[1] = v
    vector<vector<mpz_t>> switching_keys[KEY_SWITCHING_NUM][2];

   public:
    LWE(string pStr, string wStr, int nbits, string pd, int logP, bool ks) {
        this->nbits = nbits;
        this->logP = logP;
        r = Ring(pStr, wStr, nbits, pd);
        keyGen(ks);
    }

    LWE() {}

    void keyGen(bool ks) {
        // pk[0] = a, pk[1] = b;
        this->pk[0] = r.randRing();
        vector<mpz_t> e = r.randBits();
        r.ringMulPlainMod(e);
        this->sk = r.randBits();
        // b = pd * e - as
        vector<mpz_t> as = r.ringMul(this->pk[0], this->sk);
        this->pk[1] = r.ringSub(e, as);

        // generate key_switching keys
        int degree = r.getplainDegree();

        // if ks is false, then we do not need to generate key_switching keys
        if (!ks) return;

        // (1) generate gadget
        gadget = vector<mpz_t>(logP);
        for (int i = 0; i < logP; i++) {
            string exp = "1";
            exp.append(string(logP - 1 - i, '0'));
            mpz_init(gadget[logP - 1 - i]);
            mpz_set_str(gadget[logP - 1 - i], exp.c_str(), 2);
        }

        // (2) generate new keys of substitution
        for (int i = 0; i < nbits; i++) {
            substitution_keys[i] = r.subsitute(sk, (1 << nbits) / (1 << i) + 1);
        }

        // (3) generate key_switching keys
        for (int i = 0; i < nbits; i++) {
            assert(i < KEY_SWITCHING_NUM);
            switching_keys[i][0] = vector<vector<mpz_t>>(logP);
            switching_keys[i][1] = vector<vector<mpz_t>>(logP);
            ks_keyGen(substitution_keys[i], sk, switching_keys[i][0],
                      switching_keys[i][1]);
            cout << "finish ks keygen " << i << endl;
        }
    }

    int getDegree() { return r.getplainDegree(); }

    // generate key switching keys from key(s) to subkey(s')
    void ks_keyGen(vector<mpz_t>& key, vector<mpz_t>& subkey,
                   vector<vector<mpz_t>>& u, vector<vector<mpz_t>>& v) {
        for (int i = 0; i < this->logP; i++) {
            u[i] = r.randRing();
        }
        vector<vector<mpz_t>> e(this->logP);
        for (int i = 0; i < e.size(); i++) {
            e[i] = r.randBits();
            r.ringMulPlainMod(e[i]);
        }

        vector<vector<mpz_t>> gs(this->logP);
        for (int i = 0; i < this->logP; i++) {
            gs[i] = vector<mpz_t>(r.getplainDegree());
            for (int j = 0; j < r.getplainDegree(); j++) {
                mpz_init(gs[i][j]);
                mpz_mul(gs[i][j], gadget[i], key[j]);
                r.ringMod(gs[i][j]);
            }
        }

        // v = -u*s' + gadget*s + e
        for (int i = 0; i < logP; i++) {
            v[i] = r.ringMul(u[i], subkey);
            v[i] = r.ringSub(gs[i], v[i]);
            v[i] = r.ringAdd(v[i], e[i]);
        }
    }

    Ciphertext enc(Plaintext& msg) {
        Ciphertext c;
        vector<mpz_t> e0 = r.randBits();
        vector<mpz_t> e1 = r.randBits();
        r.ringMulPlainMod(e1);
        vector<mpz_t> e2 = r.randBits();
        r.ringMulPlainMod(e2);

        // u = a*e0+pd*e1
        vector<mpz_t> ae0 = r.ringMul(pk[0], e0);
        c.c[1] = r.ringAdd(ae0, e1);

        // v = b*e0+pd*e2+z
        vector<mpz_t> be0 = r.ringMul(pk[1], e0);
        vector<mpz_t> be2 = r.ringAdd(be0, e2);
        c.c[0] = r.ringAdd(be2, msg.m);
        clearRing(e0);
        clearRing(e1);
        clearRing(e2);
        clearRing(ae0);
        clearRing(be0);
        clearRing(be2);
        return c;
    }

    Plaintext dec(Ciphertext& c) {
        assert(c.num >= 2);
        Plaintext p;
        p.m = copyBigNum(c.c[0]);
        vector<mpz_t> s_pow = copyBigNum(sk);
        for (int i = 1; i < c.num; i++) {
            vector<mpz_t> sk_c = r.ringMul(s_pow, c.c[i]);
            p.m = r.ringAdd(sk_c, p.m);
            if (i != c.num - 1) {
                s_pow = r.ringMul(s_pow, sk);
            }
        }
        r.ModPd(p.m);
        return p;
    }

    // c = a+b
    Ciphertext add(Ciphertext& a, Ciphertext& b) {
        Ciphertext c;
        assert(a.num == b.num);
        c = Ciphertext(a.num);
        // TODO: change to any size additions
        for (int i = 0; i < a.num; i++) {
            c.c[i] = r.ringAdd(a.c[i], b.c[i]);
        }
        return c;
    }

    // c = a*b
    // Currently only support mul of two Ciphertext of size 2
    Ciphertext mul(Ciphertext& a, Ciphertext& b) {
        // TODO: set multiplication result *c* correctly
        assert(a.num == b.num);
        assert(a.num == 2);
        Ciphertext c(3);
        c.c[2] = r.ringMul(a.c[1], b.c[1]);
        c.c[0] = r.ringMul(a.c[0], b.c[0]);
        vector<mpz_t> vu = r.ringMul(a.c[1], b.c[0]);
        vector<mpz_t> uv = r.ringMul(a.c[0], b.c[1]);
        c.c[1] = r.ringAdd(uv, vu);
        clearRing(vu);
        clearRing(uv);
        return c;
    }

    void mul2Exp(Ciphertext& input, int exp) {
        for (int i = 0; i < input.c.size(); i++) {
            r.ringMul2Exp(input.c[i], exp);
        }
    }

    vector<mpz_t> decomposeVal(mpz_t& input) {
        vector<mpz_t> dest(logP);
        mpz_t v;
        mpz_init(v);
        mpz_set(v, input);
        bool zero = false;
        for (int i = logP - 1; i >= 0; i--) {
            mpz_init(dest[logP - 1 - i]);
            // already zero
            if (zero || (mpz_cmp_si(v, 0) == 0)) {
                mpz_set_si(dest[logP - 1 - i], 0);
                zero = true;
                continue;
            }
            // v is divisible by 2
            if (mpz_divisible_2exp_p(v, 1) != 0) {
                mpz_set_si(dest[logP - 1 - i], 0);

            } else {
                mpz_set_si(dest[logP - 1 - i], 1);
            }
            mpz_fdiv_q_2exp(v, v, 1);
        }
        return dest;
    }

    // only support keySwitch for ciphertext of size of 2
    // (c0', c1') =(c0, 0) + g ^ - 1 * K, where K =(u | v)
    void keySwitch(Ciphertext& res, vector<vector<mpz_t>>& u,
                   vector<vector<mpz_t>>& v) {
        assert(res.num == 2);

        // compute g^{-1} for c[1]
        vector<vector<mpz_t>> g_inverse(r.getplainDegree());
        for (int i = 0; i < g_inverse.size(); i++) {
            // decompose value in c[1][i]
            assert(mpz_sgn(res.c[1][i]) >= 0);
            g_inverse[i] = decomposeVal(res.c[1][i]);
        }

        // clear c[1] to all 0s
        for (int i = 0; i < r.getplainDegree(); i++) {
            mpz_set_si(res.c[1][i], 0);
        }

        for (int i = 0; i < logP; i++) {
            vector<mpz_t> gt(r.getplainDegree());
            for (int j = 0; j < r.getplainDegree(); j++) {
                mpz_init(gt[j]);
                mpz_set(gt[j], g_inverse[j][i]);
            }

            vector<mpz_t> gtu = r.ringMul(gt, u[i]);
            vector<mpz_t> gtv = r.ringMul(gt, v[i]);
            res.c[1] = r.ringAdd(gtu, res.c[1]);
            res.c[0] = r.ringAdd(gtv, res.c[0]);
        }
    }

    // only supports ciphertext of size 2
    // switched ciphertext is in res
    // it produces a ciphertext from enc(x) to enc(x^id)
    // and `res` can be decrypted with original sk
    Ciphertext substitute(Ciphertext& input, int idx) {
        Ciphertext res;
        int pow = r.getplainDegree() / (1 << idx) + 1;
        assert(input.c.size() == 2);
        assert(res.c.size() == 2);
        res.c[0] = r.subsitute(input.c[0], (1 << nbits) / (1 << idx) + 1);
        res.c[1] = r.subsitute(input.c[1], (1 << nbits) / (1 << idx) + 1);

        // start key switching
        keySwitch(res, switching_keys[idx][0], switching_keys[idx][1]);
        return res;
    }

    Ciphertext shiftCiphertext(Ciphertext& input, int shift_idx, bool neg) {
        Ciphertext res(input.num);
        for (int i = 0; i < res.num; i++) {
            res.c[i] = r.ringShift(input.c[i], shift_idx, neg);
        }
        return res;
    }

    // expand a ciphertext Enc(x^a) to 1^nbits ciphertexts where the a-th
    // ciphertext is Enc(1<<nbits) and all others are Enc(0)
    vector<Ciphertext> expand(Ciphertext& input) {
        vector<Ciphertext> res(1 << nbits);
        for (int i = 0; i < res.size(); i++) {
            res[i] = input.copy();
        }
        for (int j = 0; j < nbits; j++) {
            for (int k = 0; k < (1 << j); k++) {
                Ciphertext c1 =
                    shiftCiphertext(res[k], (1 << nbits) - (1 << j), true);
                Ciphertext c0_sub = substitute(res[k], j);
                // Ciphertext c1_sub = substitute(c1, j);
                // This shift is equivelant to substitute, but from sealPIR,
                // we only need a shift operation
                int index_raw = (((1 << nbits) - (1 << j)) *
                                 ((1 << nbits) / (1 << j) + 1)) %
                                (1 << nbits);
                Ciphertext c1_sub = shiftCiphertext(c0_sub, index_raw, false);
                res[k] = add(res[k], c0_sub);
                res[k + (1 << j)] = add(c1, c1_sub);
            }
        }
        // for (int i = 0; i < res.size(); i++) {
        //     for (int j = 0; j < res[i].c.size(); j++) {
        //         r.ringMulDegreeInv(res[i].c[j]);
        //     }
        // }
        return res;
    }
};