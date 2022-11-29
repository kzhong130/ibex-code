#include "gm.hpp"

int main(int argc, char* argv[]) {
    GM gm(1024);
    srand(time(0));
    system_clock::time_point start, end;
    start = system_clock::now();
    int times = 10000;
    for (int i = 0; i < times; i++) {
        int b1 = rand() % 2;
        int b2 = rand() % 2;
        mpz_t c1, c2, r;
        assert(gm.enc(b1, c1) == 0);
        assert(gm.enc(b2, c2) == 0);
        gm.cipherAdd(r, c1, c2);

        int dec_bit = gm.dec(r);
        assert(dec_bit == (b1 ^ b2));
    }
    end = system_clock::now();

    cout << "avg enc, add, and dec: "
         << duration_cast<std::chrono::duration<double>>(end - start).count() /
                times
         << endl;
    return 0;
}