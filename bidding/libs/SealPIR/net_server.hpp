#include <seal/seal.h>

#include <random>

#include "common.h"
#include "net.hpp"
#include "src/pir.hpp"
#include "src/pir_server.hpp"

using namespace std;
using namespace seal;
using namespace chrono;

class NetPIRServer {
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

    unique_ptr<uint8_t[]> db;

    PIRServer *server;

    bool debug = false;

   public:
    unique_ptr<uint8_t[]> db_copy;

    NetPIRServer(int total_server, int num_elem, bool debug_ = false) {
        debug = debug_;
        number_of_items = num_elem / total_server;
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

        server = new PIRServer(enc_params, pir_params);
    }

    void initDB() {
        db = make_unique<uint8_t[]>(number_of_items * size_per_item);
        if (debug) {
            db_copy = make_unique<uint8_t[]>(number_of_items * size_per_item);
        }
        cout << "db element num: " << number_of_items << endl;

        seal::Blake2xbPRNGFactory factory;
        auto gen = factory.create();
        for (uint64_t i = 0; i < number_of_items; i++) {
            for (uint64_t j = 0; j < size_per_item; j++) {
                uint8_t val = gen->generate() % 256;
                db.get()[(i * size_per_item) + j] = val;
                if (debug) {
                    db_copy.get()[(i * size_per_item) + j] = val;
                }
            }
        }
        cout << "db init finish\n";
        cerr << "db init finish\n";
    }

    bool checkAnwser(uint64_t ele_index, vector<uint8_t> &elems) {
        if (!debug) {
            cout << "debug not enabled!\n";
            return false;
        }
        for (uint32_t i = 0; i < size_per_item; i++) {
            if (elems[i] != db_copy.get()[(ele_index * size_per_item) + i]) {
                cout << "Main: elems " << (int)elems[i] << ", db "
                     << (int)db_copy.get()[(ele_index * size_per_item) + i]
                     << endl;
                cout << "Main: PIR result wrong at " << i << endl;
                return false;
            }
        }
        return true;
    }

    void load_galois_key(string str) { server->load_galois_key(str); }

    void init_process_db() {
        initDB();
        server->set_database(move(db), number_of_items, size_per_item);
        server->preprocess_database();
    }

    // deserialize query and generate reply string
    string anwserQuery(string &s) {
        stringstream client_stream, server_stream;
        system_clock::time_point start, end;
        start = system_clock::now();
        client_stream << s;
        PirQuery query = server->deserialize_query(client_stream);
        end = system_clock::now();
        cout
            << "deserialize query TIME: "
            << duration_cast<std::chrono::duration<double>>(end - start).count()
            << endl;

        cout << "query size: " << s.size() << endl;

        start = system_clock::now();
        PirReply reply = server->generate_reply(query, 0);
        end = system_clock::now();
        cout
            << "anwser query TIME: "
            << duration_cast<std::chrono::duration<double>>(end - start).count()
            << endl;

        start = system_clock::now();
        string reply_str = server->serialize_reply(reply);
        end = system_clock::now();
        cout
            << "serialize reply TIME: "
            << duration_cast<std::chrono::duration<double>>(end - start).count()
            << endl;
        return reply_str;
    }

    void worker_thread(string ip) {
        int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
        int sock_opt;
        assert(setsockopt(listen_fd, SOL_SOCKET,
                          TCP_NODELAY | SO_REUSEADDR | SO_REUSEPORT, &sock_opt,
                          sizeof(sock_opt)) >= 0);
        struct sockaddr_in servaddr = string_to_struct_addr(ip);
        assert(bind(listen_fd, (sockaddr *)&servaddr, sizeof(servaddr)) >= 0);
        assert(listen(listen_fd, 100) >= 0);

        // create and configure SSL context
        SSL_CTX *ctx = create_context();
        configure_context(ctx);

        system_clock::time_point start, end;

        while (true) {
            cout << "ready to receive from clients\n";
            // accept for one connection
            struct sockaddr_in clientaddr;
            socklen_t clientaddrlen = sizeof(clientaddr);
            int client_fd = accept(listen_fd, (struct sockaddr *)&clientaddr,
                                   &clientaddrlen);

            // establish tls connection
            SSL *ssl = SSL_new(ctx);
            SSL_set_fd(ssl, client_fd);
            assert(SSL_accept(ssl) > 0);

            // receive the galois key
            string galois_str;
            if (!recvMsg(ssl, galois_str)) {
                cout << "receive galois string fail\n";
                continue;
            }

            cout << "received galois key\n";

            // set galois key and process db
            load_galois_key(galois_str);
            init_process_db();

            cout << "process db finish, ready for query\n";
            string ack = "yes";
            assert(sendMsg(ssl, ack));

            int client_fd2 = accept(listen_fd, (struct sockaddr *)&clientaddr,
                                    &clientaddrlen);

            start = system_clock::now();
            // establish tls connection
            SSL *ssl2 = SSL_new(ctx);
            SSL_set_fd(ssl2, client_fd2);
            assert(SSL_accept(ssl2) > 0);
            end = system_clock::now();
            cout << "ssl connect TIME: "
                 << duration_cast<std::chrono::duration<double>>(end - start)
                        .count()
                 << endl;

            start = system_clock::now();
            string query_str;
            if (!recvMsg(ssl2, query_str)) {
                cout << "receive query fail\n";
                continue;
            }
            end = system_clock::now();
            cout << "ssl recv msg TIME: "
                 << duration_cast<std::chrono::duration<double>>(end - start)
                        .count()
                 << endl;

            start = system_clock::now();
            string reply_str = anwserQuery(query_str);
            end = system_clock::now();
            cout << "server process query TIME: "
                 << duration_cast<std::chrono::duration<double>>(end - start)
                        .count()
                 << endl;

            start = system_clock::now();
            assert(sendMsg(ssl2, reply_str));
            end = system_clock::now();
            cout << "ssl send msg TIME: "
                 << duration_cast<std::chrono::duration<double>>(end - start)
                        .count()
                 << endl;

            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(client_fd);

            SSL_shutdown(ssl2);
            SSL_free(ssl2);
            close(client_fd2);
        }
    }
};