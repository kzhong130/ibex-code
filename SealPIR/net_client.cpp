#include "net_client.hpp"

int main(int argc, char *argv[]) {
    string ip = "0.0.0.0:6666";

    NetPIRClient client((1 << 16));
    vector<SSL *> ssl_vec(1);
    string galois_str = client.serialize_gen_galoiskeys();
    client.init_worker(ip, ssl_vec, 0, galois_str);

    int idx = 10;
    vector<uint8_t> reply;
    client.retrieve_worker(ssl_vec[0], idx, reply);

    return 0;
}