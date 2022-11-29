#include "auction_enc.hpp"

#define BID_BIT (14)
#define BID_RANGE (1 << BID_BIT)

int main(int argc, char* argv[]) {
    AuctionEnc ae("pk", "sk", BID_BIT);
    int times = 20;
    int bid_number = atoi(argv[1]);
    double randomize_time = 0.0;
    double rand_gen_time = 0.0;
    double serialize_time = 0.0;
    double deserialize_time = 0.0;
    double dec_time = 0.0;

    cout << "bidder number: " << bid_number << endl;
    system_clock::time_point start, end;

    for (int i = 0; i < times * bid_number; i++) {
        int bid = rand() % BID_RANGE;
        vector<int> s1, s2;
        vector<mpz_t> es1, es2;
        ae.genShares(bid, es1, es2);

        start = system_clock::now();
        vector<int> r;
        ae.randMask(r);
        vector<mpz_t> er = ae.encBid(r);
        end = system_clock::now();
        rand_gen_time +=
            duration_cast<std::chrono::duration<double>>(end - start).count();

        start = system_clock::now();
        vector<mpz_t> ers1 = ae.randomize(es1, er);
        vector<mpz_t> ers2 = ae.randomize(es2, er);
        end = system_clock::now();
        randomize_time +=
            duration_cast<std::chrono::duration<double>>(end - start).count();

        start = system_clock::now();
        string ers1_str = ae.serializeShares(ers1);
        string ers2_str = ae.serializeShares(ers2);
        end = system_clock::now();
        serialize_time +=
            duration_cast<std::chrono::duration<double>>(end - start).count();

        start = system_clock::now();
        vector<mpz_t> ers1_ = ae.deserializeShares(ers1_str);
        vector<mpz_t> ers2_ = ae.deserializeShares(ers2_str);
        end = system_clock::now();
        deserialize_time +=
            duration_cast<std::chrono::duration<double>>(end - start).count();

        start = system_clock::now();
        vector<int> rs1 = ae.decBid(ers1_);
        vector<int> rs2 = ae.decBid(ers2_);
        end = system_clock::now();
        dec_time +=
            duration_cast<std::chrono::duration<double>>(end - start).count() /
            2;

        int recoverBid = ae.recoverBid(rs1, rs2);
        assert(bid == recoverBid);
    }

    cout << "(avg) randomize encrypted shares (s): " << randomize_time / times
         << endl;
    cout << "(avg) generate randomness (s): " << rand_gen_time / times << endl;
    cout << "(avg) serialize randomness (s): " << serialize_time / times
         << endl;
    cout << "(avg) deserialize randomness (s): " << deserialize_time / times
         << endl;
    cout << "(avg) decrypt encrypted shares (s): " << dec_time / times << endl;
    return 0;
}