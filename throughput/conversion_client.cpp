#include "client.hpp"
// ~2.24 MB size
#define ENC_SIZE 2355000
#define CONV_NUM 5

class BrowseClient : public Client {
    bool baseline;

    const string HTTPReqTemplate =
        "GET /index.htm HTTP/1.1"
        "User-Agent: Mozilla / 4.0(compatible; MSIE5 .01; Windows NT)"
        "Host: www.example.com"
        "Accept-Connection: Keep-Alive";

    void sendReq(string ip, vector<double>& handle_time, int id) {
        system_clock::time_point start, end;
        start = system_clock::now();
        int fd = connect_to_addr(ip);
        assert(fd > 0);
        // format HTTP request
        string HTTPReq = HTTPReqTemplate;
        for (int i = 0; i < CONV_NUM; i++) {
            if (baseline) {
                // add a random conversion number
                HTTPReq.append("conversion: 1024");
            } else {
                HTTPReq.append("conversion: ");
                HTTPReq.append(string(ENC_SIZE, 0));
            }
        }
        assert(sendMsg(fd, HTTPReq));

        // receive response
        string response;
        recvMsg(fd, response);
        end = system_clock::now();
        handle_time[id] =
            duration_cast<std::chrono::duration<double>>(end - start).count();
        cout << id << " handle time: " << handle_time[id] << endl;
        close(fd);
    }

   public:
    BrowseClient(bool b) { baseline = b; }
};

int main(int argc, char* argv[]) {
    string ip = "127.0.0.1:6667";
    int offer_load = 10;
    bool baseline = false;
    // time unit is 1 sec, so 60 means 1 min
    int time_unit = 60;
    int opt;
    while ((opt = getopt(argc, argv, "i:l:t:b")) != -1) {
        if (opt == 'l') {
            offer_load = atoi(optarg);
        } else if (opt == 'i') {
            ip = string(optarg);
        } else if (opt == 'b') {
            baseline = true;
        } else if (opt == 't') {
            time_unit = atoi(optarg);
        }
    }

    BrowseClient client(baseline);
    client.run(ip, offer_load, time_unit);
    return 0;
}