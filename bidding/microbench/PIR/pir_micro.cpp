#include <seal/seal.h>
#include <unistd.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <random>

#include "SealPIR/common.h"
#include "SealPIR/src/pir.hpp"
#include "SealPIR/src/pir_client.hpp"
#include "SealPIR/src/pir_server.hpp"

using namespace std::chrono;
using namespace std;
using namespace seal;

int main(int argc, char *argv[]) {
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
    cout << "total number of elements: " << num_elem << endl;

    // PIR related params
    uint64_t number_of_items = num_elem / total_server;
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

    EncryptionParameters enc_params(scheme_type::bfv);
    PirParams pir_params;

    // Generates all parameters

    cout << "Main: Generating SEAL parameters" << endl;
    gen_encryption_params(N, logt, enc_params);

    cout << "Main: Verifying SEAL parameters" << endl;
    verify_encryption_params(enc_params);
    cout << "Main: SEAL parameters are good" << endl;

    cout << "Main: Generating PIR parameters" << endl;
    gen_pir_params(number_of_items, size_per_item, d, enc_params, pir_params,
                   use_symmetric, use_batching, use_recursive_mod_switching);

    print_seal_params(enc_params);
    print_pir_params(pir_params);

    // Initialize PIR client....
    PIRClient client(enc_params, pir_params);
    cout << "Main: Generating galois keys for client" << endl;

    GaloisKeys galois_keys = client.generate_galois_keys();

    // Initialize PIR Server
    cout << "Main: Initializing server" << endl;
    PIRServer server(enc_params, pir_params);

    // Server maps the galois key to client 0. We only have 1 client,
    // which is why we associate it with 0. If there are multiple PIR
    // clients, you should have each client generate a galois key,
    // and assign each client an index or id, then call the procedure below.
    server.set_galois_key(0, galois_keys);

    cout << "Main: Creating the database with random data (this may take some "
            "time) ..."
         << endl;

    // Create test database
    auto db(make_unique<uint8_t[]>(number_of_items * size_per_item));

    // Copy of the database. We use this at the end to make sure we retrieved
    // the correct element.
    auto db_copy(make_unique<uint8_t[]>(number_of_items * size_per_item));

    random_device rd;
    seal::Blake2xbPRNGFactory factory;
    auto gen = factory.create();
    for (uint64_t i = 0; i < number_of_items; i++) {
        for (uint64_t j = 0; j < size_per_item; j++) {
            uint8_t val = gen->generate() % 256;
            db.get()[(i * size_per_item) + j] = val;
            db_copy.get()[(i * size_per_item) + j] = val;
        }
    }

    // Measure database setup
    auto time_pre_s = high_resolution_clock::now();
    server.set_database(move(db), number_of_items, size_per_item);
    server.preprocess_database();
    auto time_pre_e = high_resolution_clock::now();
    auto time_pre_us =
        duration_cast<duration<double>>(time_pre_e - time_pre_s).count();
    cout << "Main: database pre processed " << endl;

    // Choose an index of an element in the DB
    uint64_t ele_index =
        rd() % number_of_items;  // element in DB at random position
    uint64_t index = client.get_fv_index(ele_index);  // index of FV plaintext
    uint64_t offset =
        client.get_fv_offset(ele_index);  // offset in FV plaintext
    cout << "Main: element index = " << ele_index << " from [0, "
         << number_of_items - 1 << "]" << endl;
    cout << "Main: FV index = " << index << ", FV offset = " << offset << endl;

    // Measure query generation
    auto time_query_s = high_resolution_clock::now();
    PirQuery query = client.generate_query(index);
    auto time_query_e = high_resolution_clock::now();
    auto time_query_us =
        duration_cast<duration<double>>(time_query_e - time_query_s).count();
    cout << "Main: query generated" << endl;

    // Measure serialized query generation (useful for sending over the network)
    stringstream client_stream;
    stringstream server_stream;
    auto time_s_query_s = high_resolution_clock::now();
    int query_size = client.generate_serialized_query(index, client_stream);
    auto time_s_query_e = high_resolution_clock::now();
    auto time_s_query_us =
        duration_cast<duration<double>>(time_s_query_e - time_s_query_s)
            .count();
    cout << "Main: serialized query generated" << endl;

    // Measure query deserialization (useful for receiving over the network)
    auto time_deserial_s = high_resolution_clock::now();
    PirQuery query2 = server.deserialize_query(client_stream);
    auto time_deserial_e = high_resolution_clock::now();
    auto time_deserial_us =
        duration_cast<duration<double>>(time_deserial_e - time_deserial_s)
            .count();
    cout << "Main: query deserialized" << endl;

    // Measure query processing (including expansion)
    auto time_server_s = high_resolution_clock::now();
    // Answer PIR query from client 0. If there are multiple clients,
    // enter the id of the client (to use the associated galois key).
    PirReply reply = server.generate_reply(query2, 0);
    auto time_server_e = high_resolution_clock::now();
    auto time_server_us =
        duration_cast<duration<double>>(time_server_e - time_server_s).count();
    cout << "Main: reply generated" << endl;

    // Serialize reply (useful for sending over the network)
    string reply_str = server.serialize_reply(reply);
    PirReply reply1 = client.deserialize_reply(reply_str);

    // Measure response extraction
    auto time_decode_s = chrono::high_resolution_clock::now();
    vector<uint8_t> elems = client.decode_reply(reply1, offset);
    auto time_decode_e = chrono::high_resolution_clock::now();
    auto time_decode_us =
        duration_cast<duration<double>>(time_decode_e - time_decode_s).count();
    cout << "Main: reply decoded" << endl;

    assert(elems.size() == size_per_item);

    bool failed = false;
    // Check that we retrieved the correct element
    for (uint32_t i = 0; i < size_per_item; i++) {
        if (elems[i] != db_copy.get()[(ele_index * size_per_item) + i]) {
            cout << "Main: elems " << (int)elems[i] << ", db "
                 << (int)db_copy.get()[(ele_index * size_per_item) + i] << endl;
            cout << "Main: PIR result wrong at " << i << endl;
            failed = true;
        }
    }
    if (failed) {
        return -1;
    }

    // Output results
    cout << "[TIME] browser query generation time: " << time_query_us << " s"
         << endl;
    cout << "[TIME] browser decode PIR reply time: " << time_decode_us << " s"
         << endl;
    cout << "[TIME] bidder process one PIR query time: " << time_server_us
         << " s" << endl;
    cout << "[SIZE] One PIR query size: " << query_size / 1024.0 << " KB"
         << endl;
    cout << "[SIZE] One PIR reply size: " << reply_str.size() / 1024.0 << " KB"
         << endl;

    return 0;
}
