#include "net_server.hpp"
using namespace std;
using namespace seal;
using namespace chrono;

class PIRServers {
   private:
    // PIR related params
    uint64_t number_of_items = (1 << 16);
    int total_server = 8;

    volatile bool *galois_keys_ready;
    volatile bool *galois_keys_setup;
    volatile bool *query_ready;
    volatile bool *query_finished;
    string galois_str;
    string query_str;
    vector<string> reply_vec;

    // WARNING: this can only run once for simplicity
    void pir_server_thread(int id) {
        NetPIRServer server(total_server, number_of_items);

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
    }

    // use the reply_vec to format response reply string to client
    string format_reply() {
        string res;
        for (int i = 0; i < total_server; i++) {
            res.append(formatMsg(reply_vec[i]));
        }
        return res;
    }

   public:
    PIRServers(int total_server_, int num_elem_) {
        number_of_items = num_elem_;
        total_server = total_server_;

        // set up global variables
        galois_keys_ready = (bool *)malloc(sizeof(bool) * total_server_);
        galois_keys_setup = (bool *)malloc(sizeof(bool) * total_server_);
        query_ready = (bool *)malloc(sizeof(bool) * total_server_);
        query_finished = (bool *)malloc(sizeof(bool) * total_server_);
        reply_vec = vector<string>(total_server);

        for (int i = 0; i < total_server_; i++) {
            galois_keys_ready[i] = false;
            galois_keys_setup[i] = false;
            query_ready[i] = false;
            query_finished[i] = false;
        }

        // init pir servers thread
        for (int i = 0; i < total_server; i++) {
            thread t(&PIRServers::pir_server_thread, this, i);
            t.detach();
        }
    }

    void run_pir_servers(string ip) {
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

        // WARNING: this can only run once for simplicity
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
            if (!recvMsg(ssl, galois_str)) {
                cout << "receive galois string fail\n";
                continue;
            }

            cout << "received galois key\n";

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

            cout << "process db finish, ready for query\n";
            string ack = "yes";
            assert(sendMsg(ssl, ack));

            // establish connection

            int client_fd2 = accept(listen_fd, (struct sockaddr *)&clientaddr,
                                    &clientaddrlen);

            start = system_clock::now();
            // establish tls connection
            SSL *ssl2 = SSL_new(ctx);
            SSL_set_fd(ssl2, client_fd2);
            assert(SSL_accept(ssl2) > 0);
            end = system_clock::now();
            cout << "ssl connect time: "
                 << duration_cast<std::chrono::duration<double>>(end - start)
                        .count()
                 << endl;

            start = system_clock::now();
            if (!recvMsg(ssl2, query_str)) {
                cout << "receive query fail\n";
                continue;
            }
            end = system_clock::now();
            cout << "ssl recv msg time: "
                 << duration_cast<std::chrono::duration<double>>(end - start)
                        .count()
                 << endl;

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

            // TODO: send reply string back to client
            string reply_str = format_reply();

            start = system_clock::now();
            assert(sendMsg(ssl2, reply_str));
            end = system_clock::now();
            cout << "ssl send msg time: "
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