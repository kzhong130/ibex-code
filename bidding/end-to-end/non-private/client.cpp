#include "net.hpp"

int main(int argc, char* argv[]) {
    string auction_ip = "0.0.0.0:5555";
    auction_ip = string(argv[1]);
    cout << "auction ip: " << auction_ip << endl;

    auto start = system_clock::now();

    // contact auction server
    int fd;
    SSL* ssl = ssl_connect_to_ip(auction_ip, fd);
    string ack = "auction";
    assert(sendMsg(ssl, ack));

    // wait for winner idx from auction server
    string winner;
    assert(recvMsg(ssl, winner));
    cout << "winner is: " << winner << endl;

    auto end = system_clock::now();
    cout << "TIME: "
         << duration_cast<std::chrono::duration<double>>(end - start).count()
         << " sec" << endl;

    return 0;
}