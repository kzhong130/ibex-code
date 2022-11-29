#include <chrono>

#include "GM/auction_enc.hpp"
#include "SealPIR/net.hpp"
#include "emp-sh2pc/emp-sh2pc.h"
#include "emp-tool/emp-tool.h"
#include "iostream"

#define BIT_LEN (14)
#define LEN (BIT_LEN + 1)

using namespace emp;
using namespace std;
using namespace chrono;

#define MYASSERT(expr)            \
    if (!(expr)) {                \
        printf("%d\n", __LINE__); \
        exit(1);                  \
    }

uint32_t max(int n, vector<int> bid_vec) {
    Integer *A = new Integer[n];
    Integer *B = new Integer[n];
    Integer *Id = new Integer[n];

    for (int i = 0; i < n; ++i) {
        A[i] = Integer(LEN, bid_vec[i], ALICE);
    }

    for (int i = 0; i < n; ++i) {
        B[i] = Integer(LEN, bid_vec[i], BOB);
    }

    for (int i = 0; i < n; ++i) {
        Id[i] = Integer(7, i, PUBLIC);
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

class AuctionServer {
   public:
    int num_bid = 20;
    AuctionEnc ae;

    uint32_t auction(int party, int port, string ip_s1, vector<int> &bids) {
        NetIO *io = new NetIO(party == ALICE ? nullptr : ip_s1.c_str(), port);

        setup_semi_honest(io, party);
        uint32_t winner = max(num_bid, bids);
        finalize_semi_honest();
        delete io;
        cout << "winner: " << winner << endl;
        return winner;
    }

    void run(int party, int port, string ip_s1, string ip, int bidder_num_) {
        num_bid = bidder_num_;
        ae = AuctionEnc("pk", "sk", BIT_LEN);
        int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
        int sock_opt;
        MYASSERT(setsockopt(listen_fd, SOL_SOCKET,
                            TCP_NODELAY | SO_REUSEADDR | SO_REUSEPORT,
                            &sock_opt, sizeof(sock_opt)) >= 0);
        struct sockaddr_in servaddr = string_to_struct_addr(ip);
        MYASSERT(bind(listen_fd, (sockaddr *)&servaddr, sizeof(servaddr)) >= 0);
        if (!(listen(listen_fd, 100) >= 0)) {
            cout << "failed listen\n";
        }

        // create and configure SSL context
        SSL_CTX *ctx = create_context();
        configure_context(ctx);

        // accept for one connection
        struct sockaddr_in clientaddr;
        socklen_t clientaddrlen = sizeof(clientaddr);
        cout << "start accepting\n";
        int client_fd =
            accept(listen_fd, (struct sockaddr *)&clientaddr, &clientaddrlen);

        cout << "finish accepting " << client_fd << endl;

        MYASSERT(true);

        // establish tls connection
        SSL *ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client_fd);
        MYASSERT(SSL_accept(ssl) > 0);
        cout << "connected\n";

        // receive auction ciphertexts
        string shares_str;
        MYASSERT(recvMsg(ssl, shares_str));
        cout << "shares received size: " << shares_str.size() << endl;

        // deserialize ciphertexts
        vector<vector<mpz_t>> shares =
            ae.deserializeShareVec(shares_str, num_bid);

        vector<int> bids = ae.decodeShareVec(shares);
        // for (int i = 0; i < bids.size(); i++) {
        //     cout << i << " share" << party << ": " << bids[i] << endl;
        // }

        // run auction
        uint32_t winner = auction(party, port, ip_s1, bids);

        string winner_str = to_string(winner);
        MYASSERT(sendMsg(ssl, winner_str));

        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(client_fd);
    }
};

int main(int argc, char **argv) {
    int party = atoi(argv[1]);
    int port = atoi(argv[2]);
    string ip_s1 = string(argv[3]);
    string ip = string(argv[4]);
    int bidder_num = atoi(argv[5]);

    cout << "my ip: " << ip << endl;
    cout << "bidder num: " << bidder_num << endl;

    AuctionServer server;
    server.run(party, port, ip_s1, ip, bidder_num);

    return 0;
}