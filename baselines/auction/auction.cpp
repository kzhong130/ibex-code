#include <fstream>
#include <sstream>
#include <string>
#include <thread>

#include "net.hpp"

vector<string> readIps(string f, int bidder_num) {
    ifstream fs(f);
    vector<string> ips;
    string ip;
    for (int i = 0; i < bidder_num; i++) {
        getline(fs, ip);
        ips.push_back(ip);
        cout << ip << endl;
    }
    return ips;
}

int compute_auction(vector<int> bids) {
    int winner = 0;
    int max = bids[0];
    for (int i = 0; i < bids.size(); i++) {
        if (max < bids[i]) {
            max = bids[i];
            winner = i;
        }
    }
    return winner;
}

void contact_worker(string ip, vector<int> &bids, int id) {
    int fd;
    SSL *ssl = ssl_connect_to_ip(ip, fd);

    // send bid req
    string bidreq = "bid";
    assert(sendMsg(ssl, bidreq));

    // recv bid
    string bid;
    assert(recvMsg(ssl, bid));
    bids[id] = atoi(bid.c_str());

    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(fd);
}

vector<int> contact_bidders(vector<string> ips) {
    vector<int> bids = vector<int>(ips.size());
    vector<thread> t_vec;
    for (int i = 0; i < ips.size(); i++) {
        thread t(contact_worker, ips[i], ref(bids), i);
        t_vec.push_back(move(t));
    }
    for (auto &th : t_vec) {
        th.join();
    }
    cout << "contact bidders finish\n";
    return bids;
}

int main(int argc, char *argv[]) {
    string ip = "0.0.0.0:5555";
    string ip_file = "ips.txt";
    int bidder_num = 36;
    bidder_num = atoi(argv[1]);

    cout << "bidder num: " << bidder_num << endl;
    cout << "listen on: " << ip << endl;

    int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    int sock_opt;
    assert(setsockopt(listen_fd, SOL_SOCKET,
                      TCP_NODELAY | SO_REUSEADDR | SO_REUSEPORT, &sock_opt,
                      sizeof(sock_opt)) >= 0);
    struct sockaddr_in servaddr = string_to_struct_addr(ip);
    assert(bind(listen_fd, (sockaddr *)&servaddr, sizeof(servaddr)) >= 0);
    assert(listen(listen_fd, 100) >= 0);

    // create and configure SSL context
    SSL_CTX *ctx = create_context();
    configure_context(ctx);

    system_clock::time_point total_start, total_end;

    cout << "ready to receive from clients\n";
    // accept for one connection
    struct sockaddr_in clientaddr;
    socklen_t clientaddrlen = sizeof(clientaddr);
    int client_fd =
        accept(listen_fd, (struct sockaddr *)&clientaddr, &clientaddrlen);

    // establish tls connection with client
    SSL *ssl_client = SSL_new(ctx);
    SSL_set_fd(ssl_client, client_fd);
    assert(SSL_accept(ssl_client) > 0);

    // recv client's request for auction
    string ack;
    assert(recvMsg(ssl_client, ack));

    // contact different bidders and receive their bids
    vector<string> ips = readIps(ip_file, bidder_num);
    vector<int> bids = contact_bidders(ips);

    // compute auction
    int winner = compute_auction(bids);

    // notify the client of winner
    string winner_str = to_string(winner);
    assert(sendMsg(ssl_client, winner_str));
    cout << "max bid: " << bids[winner] << endl;
    cout << "winner idx: " << winner << endl;

    return 0;
}