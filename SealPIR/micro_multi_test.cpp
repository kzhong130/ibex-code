#include <seal/seal.h>
#include <unistd.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <random>
#include <thread>

#include "common.h"
#include "net_client.hpp"
#include "net_server.hpp"

using namespace std::chrono;
using namespace std;
using namespace seal;

volatile bool *galois_keys_ready;
volatile bool *galois_keys_setup;
volatile bool *query_ready;
volatile bool *query_finished;
volatile bool *check_output;
string galois_str;
string query_str;
vector<string> reply_vec;
vector<uint8_t> decoded_reply_vec;
int elem_id;
uint64_t offset;

void pir_server_thread(int id, int total_server, int number_of_items) {
    NetPIRServer server(total_server, number_of_items, true);

    // wait for galois keys being ready
    cout << id << " waiting for galois keys\n";
    while (galois_keys_ready[id] != true) {
    }

    server.load_galois_key(galois_str);
    server.init_process_db();
    cout << id << " finished for galois keys\n";

    galois_keys_setup[id] = true;

    cout << id << " waiting for query\n";
    // wait for query being ready
    while (query_ready[id] != true) {
    }

    reply_vec[id] = server.anwserQuery(query_str);
    query_finished[id] = true;

    cout << id << " query finish\n";

    while (check_output[id] != true) {
    }

    assert(server.checkAnwser(elem_id, decoded_reply_vec));
    cout << id << " check anwser success\n";
}

int main(int argc, char *argv[]) {
    srand(time(0));
    // the number of smaller db the bidding db is split into
    int opt;
    int log_elem = 16;
    int total_server = 8;
    while ((opt = getopt(argc, argv, "n:l:")) != -1) {
        if (opt == 'n') {
            total_server = atoi(optarg);
        } else if (opt == 'l') {
            log_elem = atoi(optarg);
        }
    }
    int num_elem = (1 << log_elem);
    cout << "number of elements: " << num_elem << endl;

    // PIR related params
    uint64_t number_of_items = num_elem;
    uint64_t size_per_item = SIZE_ELEM;  // in bytes
    uint32_t N = N_VAL;

    // Recommended values: (logt, d) = (20, 2).
    uint32_t logt = LOGT;
    uint32_t d = D_VAL;
    bool use_symmetric = true;  // use symmetric encryption instead of public
                                // key (recommended for smaller query)
    bool use_batching = true;   // pack as many elements as possible into a BFV
                                // plaintext (recommended)
    bool use_recursive_mod_switching = true;

    NetPIRClient client(number_of_items, total_server);

    galois_keys_ready = (bool *)malloc(sizeof(bool) * total_server);
    galois_keys_setup = (bool *)malloc(sizeof(bool) * total_server);
    query_ready = (bool *)malloc(sizeof(bool) * total_server);
    query_finished = (bool *)malloc(sizeof(bool) * total_server);
    check_output = (bool *)malloc(sizeof(bool) * total_server);
    reply_vec = vector<string>(total_server);

    for (int i = 0; i < total_server; i++) {
        galois_keys_ready[i] = false;
        galois_keys_setup[i] = false;
        query_ready[i] = false;
        query_finished[i] = false;
        check_output[i] = false;
    }

    for (int i = 0; i < total_server; i++) {
        thread t(pir_server_thread, i, total_server, number_of_items);
        t.detach();
    }

    galois_str = client.serialize_gen_galoiskeys();
    // distribute galois keys to all worker threads
    vector<int> waitlist_galois;
    for (int i = 0; i < total_server; i++) {
        galois_keys_ready[i] = true;
        waitlist_galois.push_back(i);
    }

    while (!waitlist_galois.empty()) {
        for (int i = 0; i < waitlist_galois.size(); i++) {
            if (galois_keys_setup[waitlist_galois[i]]) {
                waitlist_galois.erase(waitlist_galois.begin() + i);
                break;
            }
        }
    }

    // element id in all db
    int elem_idx_all = rand() % num_elem;
    elem_id = elem_idx_all % (number_of_items / total_server);
    cout << "the actual element id: " << elem_idx_all
         << ", and query id: " << elem_id << endl;
    query_str = client.generateQuery(elem_id, offset);

    // distribute query to all server threads
    vector<int> waitlist_query;
    for (int i = 0; i < total_server; i++) {
        query_ready[i] = true;
        waitlist_query.push_back(i);
    }

    while (!waitlist_query.empty()) {
        for (int i = 0; i < waitlist_query.size(); i++) {
            if (query_finished[waitlist_query[i]]) {
                waitlist_query.erase(waitlist_query.begin() + i);
                break;
            }
        }
    }

    cout << "finish answering query\n";

    int server_id = elem_idx_all / (num_elem / total_server);
    cout << "element idx: " << elem_idx_all << " belong to " << server_id
         << " server" << endl;

    // decode reply
    decoded_reply_vec = client.recoverReply(reply_vec[server_id], offset);

    check_output[server_id] = true;

    sleep(3);
    return 0;
}
