#include "gm.hpp"

int main() {
    GM gm(1024);
    gm.output_keys("pk", "sk");

    GM test_gm("pk", "sk");

    for (int i = 0; i < 100; i++) {
        int bit = rand() % 2;
        mpz_t v;
        mpz_init(v);
        gm.enc(bit, v);
        assert(test_gm.dec(v) == bit);
    }
    return 0;
}