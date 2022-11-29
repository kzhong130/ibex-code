#include <random>

#include "server.hpp"
#define COOKIE_LEN 4096
// Response page of 1MB
#define PAGE_SIZE 1048576

class BrowseServer : public Server {
    const string alphanum =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    const string HTTPRes =
        "HTTP/1.1 200 OK"
        "Date: Mon, 27 Jul 2009 12:28:53 GMT"
        "Server: Apache/2.2.14 (Win32)"
        "Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT"
        "Content-Length: 88"
        "Content-Type: text/html";

    string genResponse() {
        string s = HTTPRes;
        // format cookie
        s += "Cookie: ";
        for (int i = 0; i < COOKIE_LEN; ++i) {
            s += alphanum.c_str()[rand() % (alphanum.size() - 1)];
        }
        // append a random web page content
        s.append(string(PAGE_SIZE, 0));
        return s;
    }

    void handle(string msg, int fd) {
        string response = genResponse();
        assert(sendMsg(fd, response));
    }
};

int main(int argc, char* argv[]) {
    string ip = "0.0.0.0:6667";
    // fixed 60 sec
    double time_period = 60.0;
    int opt;
    while ((opt = getopt(argc, argv, "i:t:")) != -1) {
        if (opt == 'i') {
            ip = string(optarg);
        } else if (opt == 't') {
            time_period = atof(optarg);
        }
    }
    BrowseServer server;
    server.run(ip, time_period);
    return 0;
}