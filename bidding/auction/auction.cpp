#include <chrono>

#include "emp-sh2pc/emp-sh2pc.h"
#include "emp-tool/emp-tool.h"
#include "iostream"

// one bit needs for the sign...
#define LEN (14 + 1)

using namespace emp;
using namespace std;
using namespace chrono;

uint32_t max(int n, vector<int> bid_vec) {
    Integer *A = new Integer[n];
    Integer *B = new Integer[n];
    Integer *Id = new Integer[n];

    for (int i = 0; i < n; ++i) {
        Id[i] = Integer(7, i, PUBLIC);
    }

    for (int i = 0; i < n; ++i) {
        A[i] = Integer(LEN, bid_vec[i], ALICE);
    }

    for (int i = 0; i < n; ++i) {
        B[i] = Integer(LEN, bid_vec[i], BOB);
    }

    Integer *C = new Integer[n];
    for (int i = 0; i < n; ++i) {
        C[i] = (A[i] ^ B[i]);
    }

    Integer max = C[0];
    std::cout << max.size() << std::endl;
    Integer maxId = Id[0];
    for (int i = 0; i < n; ++i) {
        Bit cond = C[i].geq(max);
        // x.select(b, y) -> if b: y else: x
        max = max.select(cond, C[i]);
        maxId = maxId.select(cond, Id[i]);
    }

    uint32_t winner = maxId.reveal<uint32_t>();
    cout << "max: " << max.reveal<uint32_t>() << endl;
    cout << "maxid: " << winner << endl;
    return winner;
}

int main(int argc, char **argv) {
    int port, party;
    parse_party_and_port(argv, &party, &port);

    system_clock::time_point start, end;
    start = system_clock::now();
    vector<int> bids;
    int num_bid = atoi(argv[3]);
    for (int i = 0; i < num_bid; i++) {
        bids.push_back(i);
    }
    cout << "number of bids: " << num_bid << endl;
    NetIO* io = new NetIO(party == ALICE ? nullptr : "127.0.0.1", port);

    setup_semi_honest(io, party);
    uint32_t id = max(num_bid, bids);
    finalize_semi_honest();
    cout << "winner id: " << id << endl;
    end = system_clock::now();
    cout << "TIME: "
         << duration_cast<std::chrono::duration<double>>(end - start).count()
         << endl;
    delete io;
}