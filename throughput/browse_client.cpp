#include "client.hpp"
// ~2.24 MB size
// #define ENC_SIZE 2355000

class BrowseClient : public Client {
    bool baseline;
    int enc_size;

    const string HTTPReqTemplate =
        "GET /index.htm HTTP/1.1"
        "User-Agent: Mozilla / 4.0(compatible; MSIE5 .01; Windows NT)"
        "Host: www.example.com"
        "Accept-Connection: Keep-Alive"
        "Cohort: ";

    void sendReq(string ip, vector<double>& handle_time, int id) {
        system_clock::time_point start, end;
        start = system_clock::now();
        int fd = connect_to_addr(ip);
        assert(fd > 0);
        // format HTTP request
        string HTTPReq = HTTPReqTemplate;
        if (baseline) {
            // add a random cohort number
            HTTPReq.append("1024");
        } else {
            HTTPReq.append(string(enc_size, 0));
        }
        assert(sendMsg(fd, HTTPReq));

        // receive cookie as response
        string cookie;
        recvMsg(fd, cookie);
        end = system_clock::now();
        handle_time[id] =
            duration_cast<std::chrono::duration<double>>(end - start).count();
        cout << id << " handle time: " << handle_time[id] << endl;
        close(fd);
    }

   public:
    BrowseClient(bool b, int e) {
        baseline = b;
        enc_size = e;
    }
};

int main(int argc, char* argv[]) {
    string ip = "127.0.0.1:6667";
    int offer_load = 10;
    bool baseline = false;
    // time unit is 1 sec, so 60 means 1 min
    int time_unit = 60;
    // MB
    int opt;
    int enc_size = 2.24 * 1024 * 1024;
    while ((opt = getopt(argc, argv, "i:l:t:s:b")) != -1) {
        if (opt == 'l') {
            offer_load = atoi(optarg);
        } else if (opt == 'i') {
            ip = string(optarg);
        } else if (opt == 'b') {
            baseline = true;
        } else if (opt == 't') {
            time_unit = atoi(optarg);
        } else if (opt == 's') {
            enc_size = atof(optarg) * 1024 * 1024;
        }
    }
    cout << "encrypted share size (bytes): " << enc_size << endl;
    BrowseClient client(baseline, enc_size);
    client.run(ip, offer_load, time_unit);
    return 0;
}