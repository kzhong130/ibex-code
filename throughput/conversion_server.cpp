#include <random>

#include "server.hpp"

class BrowseServer : public Server {
    const string HTTPRes =
        "HTTP/1.1 200 OK"
        "Date: Mon, 27 Jul 2009 12:28:53 GMT"
        "Server: Apache/2.2.14 (Win32)"
        "Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT"
        "Content-Length: 88"
        "Content-Type: text/html";

    string genResponse() { return HTTPRes; }

    void handle(string msg, int fd) {
        string response = genResponse();
        assert(sendMsg(fd, response));
    }
};

int main(int argc, char* argv[]) {
    string ip = "0.0.0.0:6667";
    // fixed 1 sec
    double time_period = 1.0;
    int opt;
    while ((opt = getopt(argc, argv, "i:")) != -1) {
        if (opt == 'i') {
            ip = string(optarg);
        }
    }
    BrowseServer server;
    server.run(ip, time_period);
    return 0;
}