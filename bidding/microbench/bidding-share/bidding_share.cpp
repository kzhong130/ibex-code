#include "GM/auction_enc.hpp"

#define BID_BIT (14)
#define BID_RANGE (1 << BID_BIT)

int main(int argc, char* argv[]) {
    AuctionEnc ae("pk", "sk", BID_BIT);
    int times = 20;
    int bid_number = atoi(argv[1]);
    double randomize_time = 0.0;
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
        randomize_time +=
            duration_cast<std::chrono::duration<double>>(end - start).count();

        start = system_clock::now();
        vector<mpz_t> ers1 = ae.randomize(es1, er);
        vector<mpz_t> ers2 = ae.randomize(es2, er);
        end = system_clock::now();
        randomize_time +=
            duration_cast<std::chrono::duration<double>>(end - start).count();

        string ers1_str = ae.serializeShares(ers1);
        string ers2_str = ae.serializeShares(ers2);

        start = system_clock::now();
        vector<mpz_t> ers1_ = ae.deserializeShares(ers1_str);
        vector<mpz_t> ers2_ = ae.deserializeShares(ers2_str);
        end = system_clock::now();
        dec_time +=
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

    cout << "(avg) browser randomizes encrypted shares (ms): "
         << randomize_time / times * 1000.0 << endl;
    cout << "(avg) auction server decrypts encrypted shares (ms): "
         << dec_time / times * 1000.0 << endl;
    cout << "bidding shares: " << 2048.0 / 8 / 1024 * bid_number * BID_BIT
         << " KB" << endl;
    return 0;
}