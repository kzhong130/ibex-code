#include <fstream>
#include <sstream>
#include <string>
#include <thread>

#include "net.hpp"

int main(int argc, char *argv[]) {
    string ip = "0.0.0.0:6666";
    ip = string(argv[1]);

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

    int port = atoi(split(ip, ":")[1].c_str());
    srand(time(0) + port);
    int bid = rand() % 1024;
    cout << "bid: " << bid << endl;

    // notify the client of winner
    string bid_str = to_string(bid);
    assert(sendMsg(ssl_client, bid_str));

    return 0;
}