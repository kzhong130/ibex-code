#include <thread>

#include "GM/auction_enc.hpp"
#include "SealPIR/net_client.hpp"

#define BID_BIT (14)
#define BID_RANGE (1 << BID_BIT)
#define SK_BIT (1024)

class Client {
   private:
    vector<string> ips;
    string auction_ip_1;
    string auction_ip_2;
    NetPIRClient client;
    AuctionEnc ae;

    int pir_server_num;
    int bidder_num;
    int num_elem;

    volatile bool* send_query_ready;
    volatile bool* recover_ready;

    void readIps(string f) {
        ifstream fs(f);
        string ip;
        getline(fs, auction_ip_1);
        getline(fs, auction_ip_2);
        cout << auction_ip_1 << " " << auction_ip_2 << endl;
        assert(bidder_num != 0);
        for (int i = 0; i < bidder_num; i++) {
            getline(fs, ip);
            ips.push_back(ip);
            cout << ip << endl;
        }
    }

   public:
    Client(string iplist, int pir_server_num_, int bidder_num_, int num_elem_) {
        bidder_num = bidder_num_;
        pir_server_num = pir_server_num_;
        num_elem = num_elem_;
        readIps(iplist);

        // init variable
        send_query_ready = (bool*)malloc(sizeof(bool) * bidder_num);
        recover_ready = (bool*)malloc(sizeof(bool) * bidder_num);

        for (int i = 0; i < bidder_num; i++) {
            send_query_ready[i] = false;
            recover_ready[i] = false;
        }

        client = NetPIRClient(num_elem_, pir_server_num);
        // For simplicity, we use the same set of (pk, sk),
        // for two auction servers
        ae = AuctionEnc("pk", "sk", BID_BIT);
    }

    void init_pir_server_worker(string ip, vector<SSL*>& ssl_vec, int id,
                                string& galois_str) {
        client.init_worker(ip, ssl_vec, id, galois_str);
    }

    void connect_pir_servers() {
        vector<SSL*> ssl_vec(ips.size());
        vector<thread> t_vec;
        string galois_str = client.serialize_gen_galoiskeys();
        for (int i = 0; i < ips.size(); i++) {
            thread t(&Client::init_pir_server_worker, this, ips[i],
                     ref(ssl_vec), i, ref(galois_str));
            t_vec.push_back(move(t));
        }

        for (auto& th : t_vec) {
            th.join();
        }
        cout << "init with all pir servers\n";
    }

    void recoverReply_multi(string& replyStr, uint64_t offset, int elem_id) {
        int server_id = elem_id / (num_elem / pir_server_num);
        char* ptr = (char*)replyStr.c_str();
        uint32_t res_len = 0;

        for (int i = 0; i < server_id; i++) {
            ptr += res_len;
            res_len = *((uint32_t*)ptr);
            res_len = ntohl(res_len);
            ptr += 4;
        }
        string reply_to_decode = string(ptr, res_len);
        vector<uint8_t> reply = client.recoverReply(reply_to_decode, offset);
    }

    void query_pir_server_worker(string& queryStr, string& ip, uint64_t offset,
                                 int elem_id, int id) {
        while (send_query_ready[id] != true) {
        }
        int fd;
        system_clock::time_point start, end;
        start = system_clock::now();
        SSL* ssl = ssl_connect_to_ip(ip, fd);
        end = system_clock::now();
        cout
            << id << " QUERY PIR: ssl connection TIME: "
            << duration_cast<std::chrono::duration<double>>(end - start).count()
            << endl;

        start = system_clock::now();
        assert(sendMsg(ssl, queryStr));
        end = system_clock::now();
        cout
            << id << " QUERY PIR: send message TIME: "
            << duration_cast<std::chrono::duration<double>>(end - start).count()
            << endl;
        cout << "sent query size: " << queryStr.size() << endl;

        start = system_clock::now();
        string replyStr;
        assert(recvMsg(ssl, replyStr));
        end = system_clock::now();
        cout
            << id << " QUERY PIR: receive reply TIME: "
            << duration_cast<std::chrono::duration<double>>(end - start).count()
            << endl;
        cout << "received reply size: " << replyStr.size() << endl;

        start = system_clock::now();
        recoverReply_multi(replyStr, offset, elem_id);
        end = system_clock::now();
        cout
            << id << " QUERY PIR: recover reply TIME: "
            << duration_cast<std::chrono::duration<double>>(end - start).count()
            << endl;
        recover_ready[id] = true;
        cout << "finish query\n";
    }

    void generate_pir_queries(int elem_id, vector<uint64_t>& offsets,
                              vector<string>& queries) {
        queries = vector<string>(ips.size());
        offsets = vector<uint64_t>(ips.size());
        int query_id = elem_id % (num_elem / pir_server_num);
        for (int i = 0; i < ips.size(); i++) {
            queries[i] = client.generateQuery(query_id, offsets[i]);
        }
    }

    void query_pir_servers(vector<string>& query_vec, vector<uint64_t>& offsets,
                           int idx) {
        // vector<thread> t_vec;
        for (int i = 0; i < ips.size(); i++) {
            thread t(&Client::query_pir_server_worker, this, ref(query_vec[i]),
                     ref(ips[i]), offsets[i], idx, i);
            t.detach();
            // t_vec.push_back(move(t));
        }
        // for (auto& th : t_vec) {
        //     th.join();
        // }
        cout << "finish setting up querying pir servers\n";
    }

    void send_auction_shares_worker(string& str, string ip, SSL*& ssl,
                                    int& fd) {
        ssl = ssl_connect_to_ip(ip, fd);
        assert(sendMsg(ssl, str));
        cout << "finish sending: " << str.size() << " bytes" << endl;
    }

    uint32_t connect_auction_servers(vector<vector<mpz_t>>& s1,
                                     vector<vector<mpz_t>>& s2) {
        // serialize s1, s2
        string s1_str = ae.serializeShareVec(s1, bidder_num);
        string s2_str = ae.serializeShareVec(s2, bidder_num);

        int fd1, fd2;
        SSL* ssl1;
        SSL* ssl2;
        thread t1(&Client::send_auction_shares_worker, this, ref(s1_str),
                  auction_ip_1, ref(ssl1), ref(fd1));
        thread t2(&Client::send_auction_shares_worker, this, ref(s2_str),
                  auction_ip_2, ref(ssl2), ref(fd2));
        t1.join();
        t2.join();

        cout << "connected to auction servers\n";
        string winner;
        assert(recvMsg(ssl1, winner));
        cout << "winner: " << winner << endl;
        return atoi(winner.c_str());
    }

    void run() {
        srand(time(0));
        // init with the PIR servers
        connect_pir_servers();

        // prepare pir requests
        int elem_idx = rand() % num_elem;
        vector<string> query_vec;
        vector<uint64_t> offsets;
        generate_pir_queries(elem_idx, offsets, query_vec);

        // init some bids
        // Note: for simplicity, we ask the client to generate some encrypted
        // bids, instead of taking time to init the db on pir servers.
        vector<int> bids(bidder_num);
        vector<vector<mpz_t>> encShare1(bidder_num);
        vector<vector<mpz_t>> encShare2(bidder_num);
        for (int i = 0; i < bidder_num; i++) {
            bids[i] = rand() % BID_RANGE;
            cout << i << " bid: " << bids[i] << endl;
            ae.genShares(bids[i], encShare1[i], encShare2[i]);
        }

        // spawn threads
        query_pir_servers(query_vec, offsets, elem_idx);

        // read from PIR servers
        system_clock::time_point start, end, s_, e_;
        start = system_clock::now();

        s_ = system_clock::now();
        // set query threads to start sending requests
        vector<int> waitlist_query_threads;
        for (int i = 0; i < bidder_num; i++) {
            send_query_ready[i] = true;
            waitlist_query_threads.push_back(i);
        }

        // wait for query threads to finish
        while (!waitlist_query_threads.empty()) {
            for (int i = 0; i < waitlist_query_threads.size(); i++) {
                if (recover_ready[waitlist_query_threads[i]]) {
                    waitlist_query_threads.erase(
                        waitlist_query_threads.begin() + i);
                    break;
                }
            }
        }
        e_ = system_clock::now();
        double query_pir_time =
            duration_cast<std::chrono::duration<double>>(e_ - s_).count();

        cout << "pir time: " << query_pir_time << endl;

        // randomize the shares
        for (int i = 0; i < bidder_num; i++) {
            vector<int> r;
            ae.randMask(r);
            vector<mpz_t> er = ae.encBid(r);
            encShare1[i] = ae.randomize(encShare1[i], er);
            encShare2[i] = ae.randomize(encShare2[i], er);
        }

        s_ = system_clock::now();
        // submit to auction servers and receive response, all done!
        uint32_t winner = connect_auction_servers(encShare1, encShare2);
        e_ = system_clock::now();
        double auction_time =
            duration_cast<std::chrono::duration<double>>(e_ - s_).count();

        end = system_clock::now();
        cout
            << "TIME: "
            << duration_cast<std::chrono::duration<double>>(end - start).count()
            << endl;

        cout << "winner bid: " << bids[winner] << endl;
        cout << "auction time: " << auction_time << endl;
    }
};

int main(int argc, char* argv[]) {
    string file = "ips.txt";
    int bidder_num = 30;
    int pir_server_num = 8;
    int opt;
    int log_elem = 16;
    while ((opt = getopt(argc, argv, "n:b:l:")) != -1) {
        if (opt == 'n') {
            pir_server_num = atoi(optarg);
        } else if (opt == 'b') {
            bidder_num = atoi(optarg);
        } else if (opt == 'l') {
            log_elem = atoi(optarg);
        }
    }
    int num_elem = (1 << log_elem);
    cout << "number of elements in db: " << num_elem << endl;
    Client client(file, pir_server_num, bidder_num, num_elem);
    client.run();
    return 0;
}