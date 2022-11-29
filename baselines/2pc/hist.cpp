#include "chrono"
#include "emp-sh2pc/emp-sh2pc.h"
using namespace emp;
using namespace std;
using namespace chrono;

#define NUM 20

void test_hist(int party, int number, int portion, int targetId, int nbits) {
    int n = 1 << nbits;
    Integer tid(20, targetId, ALICE);
    Integer* s1 = new Integer[portion];
    Integer* s2 = new Integer[portion];
    Integer* features = new Integer[portion];
    for (int i = 0; i < portion; i++) {
        s1[i] = Integer(20, number, ALICE);
    }
    for (int i = 0; i < portion; i++) {
        s2[i] = Integer(20, number, BOB);
    }
    for (int i = 0; i < portion; i++) {
        features[i] = s1[i] + s2[i];
    }

    Integer* targetIds = new Integer[portion];
    for (int i = 0; i < portion; i++) {
        targetIds[i] = Integer(20, i, PUBLIC);
    }
    Integer* res = new Integer[n];
    Integer* id = new Integer[n];
    for (int i = 0; i < n; i++) {
        id[i] = Integer(20, i, PUBLIC);
        res[i] = Integer(20, 0, PUBLIC);
    }
    Integer zero(20, 0, PUBLIC);
    Integer one(20, 1, PUBLIC);
    for (int j = 0; j < portion; j++) {
        for (int i = 0; i < n; i++) {
            Bit isTarget = tid.equal(targetIds[j]);
            Bit isBucket = features[j].equal(id[i]);
            Bit cond = isTarget & isBucket;
            // x.select(b, y) -> if b: y else: x
            Integer v = zero.select(cond, one);
            res[i] = res[i] + v;
        }
    }
    int randId = rand() % n;
    while (randId == 2 * NUM) {
        randId = rand() % n;
    }
    cout << "aggregate target should be 1: " << res[2 * NUM].reveal<uint32_t>()
         << endl;
    cout << "aggregate target for random index " << randId
         << " should be 0: " << res[randId].reveal<uint32_t>() << endl;
}

int main(int argc, char** argv) {
    int port, party;
    parse_party_and_port(argv, &party, &port);
    srand(time(0));
    int portion = 100;
    int targetId = rand() % portion;
    int nbits = 16;
    if (argc > 3) nbits = atoi(argv[3]);
    if (argc > 4) portion = atoi(argv[4]);
    system_clock::time_point start, end;
    start = system_clock::now();

    NetIO* io = new NetIO(party == ALICE ? nullptr : "127.0.0.1", port);

    setup_semi_honest(io, party);
    test_hist(party, NUM, portion, targetId, nbits);
    finalize_semi_honest();
    end = system_clock::now();
    cout << "TIME: "
         << duration_cast<std::chrono::duration<double>>(end - start).count()
         << endl;
    delete io;
}