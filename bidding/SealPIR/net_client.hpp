#include <seal/seal.h>

#include "common.h"
#include "net.hpp"
#include "src/pir.hpp"
#include "src/pir_client.hpp"

using namespace std;
using namespace seal;
using namespace chrono;

class NetPIRClient {
   private:
    // PIR related params
    uint64_t number_of_items = (1 << 16);
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

    PIRClient *client;

   public:
    NetPIRClient() {}

    NetPIRClient(int num_elem_, int server_num = 8) {
        number_of_items = num_elem_ / server_num;
        EncryptionParameters enc_params(scheme_type::bfv);
        PirParams pir_params;
        cout << "Main: Generating SEAL parameters" << endl;
        gen_encryption_params(N, logt, enc_params);

        cout << "Main: Verifying SEAL parameters" << endl;
        verify_encryption_params(enc_params);
        cout << "Main: SEAL parameters are good" << endl;

        cout << "Main: Generating PIR parameters" << endl;
        gen_pir_params(number_of_items, size_per_item, d, enc_params,
                       pir_params, use_symmetric, use_batching,
                       use_recursive_mod_switching);

        client = new PIRClient(enc_params, pir_params);
    }

    string generateQuery(int ele_index, uint64_t &offset) {
        uint64_t index =
            client->get_fv_index(ele_index);        // index of FV plaintext
        offset = client->get_fv_offset(ele_index);  // offset in FV plaintext

        PirQuery query = client->generate_query(index);
        stringstream client_stream;
        int query_size =
            client->generate_serialized_query(index, client_stream);
        return client_stream.str();
    }

    vector<uint8_t> recoverReply(string s, uint64_t offset) {
        stringstream ss;
        ss << s;
        PirReply reply = client->deserialize_reply(s);
        vector<uint8_t> elems = client->decode_reply(reply, offset);
        return elems;
    }

    string serialize_gen_galoiskeys() {
        return client->serialize_gen_galoiskeys();
    }

    // initialize connection and send galois key
    void init_worker(string ip, vector<SSL *> &ssl_vec, int id,
                     string &galois_str) {
        const SSL_METHOD *method = TLS_client_method();

        SSL_CTX *ctx = SSL_CTX_new(method);
        assert(ctx != NULL);
        SSL *ssl = SSL_new(ctx);
        int fd = connect_to_addr(ip);
        SSL_set_fd(ssl, fd);
        assert(SSL_connect(ssl) == 1);

        // send galois key
        assert(sendMsg(ssl, galois_str));

        // receive ack
        string ack;
        assert(recvMsg(ssl, ack));
        cout << "recv ack from server\n";

        ssl_vec[id] = ssl;
    }

    void retrieve_worker(SSL *ssl, uint64_t idx, vector<uint8_t> &reply) {
        uint64_t offset;

        // send query
        string query_str = generateQuery(idx, offset);
        assert(sendMsg(ssl, query_str));

        // receive response
        string reply_str;
        assert(recvMsg(ssl, reply_str));

        // recover reply
        reply = recoverReply(reply_str, offset);
    }
};