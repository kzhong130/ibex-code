#include "he.hpp"

#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    int opt;
    int log_degree = 15;
    while ((opt = getopt(argc, argv, "d:")) != -1) {
        if (opt == 'd') {
            log_degree = atoi(optarg);
        }
    }
    HE he(log_degree);
    for (int i = 0; i < 100; i++) {
        int s1 = rand() % (1 << log_degree);
        int s2 = rand() % (1 << log_degree);
        Ciphertext c, dst;
        he.encShare(s1, c);
        he.shift(c, s2, dst);
        Plaintext p;
        he.dec(dst, p);
        string poly = "1x^";
        poly.append(to_string(s1 + s2));
        Plaintext expected = Plaintext(poly);
        assert(p == expected);
        assert(p.to_string() == expected.to_string());
        cout << "passed " << i << " test\n";
    }
    cout << "aggregation limit: " << he.aggLimit() << endl;
    return 0;
}