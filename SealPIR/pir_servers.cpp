#include "pir_servers.hpp"

int main(int argc, char *argv[]) {
    string ip = "0.0.0.0:6666";
    int total_server = 8;
    int opt;
    int log_elem = 16;
    while ((opt = getopt(argc, argv, "i:n:l:")) != -1) {
        if (opt == 'n') {
            total_server = atoi(optarg);
        } else if (opt == 'i') {
            ip = string(optarg);
        } else if (opt == 'l') {
            log_elem = atoi(optarg);
        }
    }
    int num_elem = (1 << log_elem);
    cout << "number of elements in a pir server: " << num_elem << endl;
    PIRServers server(total_server, num_elem);
    server.run_pir_servers(ip);
    return 0;
}