#include "gm.hpp"

class AuctionEnc {
   private:
    GM gm;
    int bit_range;

   public:
    AuctionEnc() {}

    AuctionEnc(int sk_bit, int br_) {
        bit_range = br_;
        srand(time(0));
        gm = GM(sk_bit);
    }

    AuctionEnc(string pk_file, string sk_file, int br_) {
        bit_range = br_;
        srand(time(0));
        gm = GM(pk_file, sk_file);
    }

    vector<mpz_t> encBid(vector<int>& bit_vec) {
        vector<mpz_t> enc_vec(bit_vec.size());
        for (int i = 0; i < bit_vec.size(); i++) {
            assert(gm.enc(bit_vec[i], enc_vec[i]) == 0);
        }
        return enc_vec;
    }

    vector<int> decBid(vector<mpz_t>& v) {
        vector<int> dec_vec(v.size());
        for (int i = 0; i < v.size(); i++) {
            dec_vec[i] = gm.dec(v[i]);
            assert(dec_vec[i] != -1);
        }
        return dec_vec;
    }

    vector<mpz_t> randomize(vector<mpz_t>& v, vector<mpz_t>& r) {
        assert(v.size() == r.size());
        vector<mpz_t> res(v.size());
        for (int i = 0; i < r.size(); i++) {
            gm.cipherAdd(res[i], v[i], r[i]);
        }
        return res;
    }

    void splitBid(int bid, vector<int>& s1, vector<int>& s2) {
        int mask = 1;
        for (int i = 0; i < bit_range; i++) {
            int b = bid & mask;
            bid >>= 1;
            int b1 = rand() % 2;
            s1.push_back(b1);
            s2.push_back(b ^ b1);
        }
    }

    int recoverBid(vector<int>& s1, vector<int>& s2) {
        int mask = 1;
        int bid = 0;
        for (int i = 0; i < s1.size(); i++) {
            bid = (bid | ((s1[i] ^ s2[i]) << i));
        }
        return bid;
    }

    void randMask(vector<int>& r) {
        for (int i = 0; i < bit_range; i++) {
            int rb = rand() % 2;
            r.push_back(rb);
        }
    }

    string serializeShares(vector<mpz_t>& c) {
        string res;
        for (int i = 0; i < bit_range; i++) {
            string c_str = gm.serialize_mpz(c[i]);
            res.append(c_str);
        }
        return res;
    }

    vector<mpz_t> deserializeShares(string str) {
        vector<mpz_t> res(bit_range);
        char* ptr = (char*)str.c_str();
        for (int i = 0; i < bit_range; i++) {
            mpz_init(res[i]);
            int mv_len = gm.deserialize_mpz(ptr, res[i]);
            ptr += mv_len;
        }
        return res;
    }

    void genShares(int bid, vector<mpz_t>& e1, vector<mpz_t>& e2) {
        vector<int> s1, s2;
        splitBid(bid, s1, s2);
        e1 = encBid(s1);
        e2 = encBid(s2);
    }

    string serializeShareVec(vector<vector<mpz_t>>& c, int bidder_num) {
        assert(c.size() == bidder_num);
        string res;
        for (int i = 0; i < bidder_num; i++) {
            for (int j = 0; j < bit_range; j++) {
                string c_str = gm.serialize_mpz(c[i][j]);
                res.append(c_str);
            }
        }
        return res;
    }

    vector<vector<mpz_t>> deserializeShareVec(string str, int bidder_num) {
        vector<vector<mpz_t>> c(bidder_num);
        char* ptr = (char*)str.c_str();
        for (int i = 0; i < bidder_num; i++) {
            c[i] = vector<mpz_t>(bit_range);
            for (int j = 0; j < bit_range; j++) {
                mpz_init(c[i][j]);
                int mv_pos = gm.deserialize_mpz(ptr, c[i][j]);
                ptr += mv_pos;
            }
        }
        return c;
    }

    vector<int> decodeShareVec(vector<vector<mpz_t>>& v) {
        vector<int> shares(v.size());
        for (int i = 0; i < v.size(); i++) {
            int mask = 1;
            int bid = 0;
            vector<int> bits = decBid(v[i]);
            for (int j = 0; j < bits.size(); j++) {
                bid = (bid | (bits[j] << j));
            }
            shares[i] = bid;
        }
        return shares;
    }
};