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
        client = NetPIRClient(num_elem_, pir_server_num);
        // For simplicity, we use the same set of (pk, sk),
        // for two auction servers
        ae = AuctionEnc("pk", "sk", BID_BIT);
    }

    void init_pir_server_worker(string ip, vector<SSL*>& ssl_vec, int id,
                                string& galois_str) {
        client.init_worker(ip, ssl_vec, id, galois_str);
    }

    vector<string> connect_pir_servers() {
        vector<SSL*> ssl_vec(ips.size() * pir_server_num);
        vector<string> pir_ips;
        vector<thread> t_vec;
        string galois_str = client.serialize_gen_galoiskeys();
        for (int i = 0; i < ips.size(); i++) {
            vector<string> addr_and_port = split(ips[i], ":");
            int port = atoi(addr_and_port[1].c_str());
            for (int j = 0; j < pir_server_num; j++) {
                string ip = addr_and_port[0];
                ip.append(":");
                ip.append(to_string(port + j));
                cout << ip << endl;
                pir_ips.push_back(ip);
                thread t(&Client::init_pir_server_worker, this, ip,
                         ref(ssl_vec), i * pir_server_num + j, ref(galois_str));
                t_vec.push_back(move(t));
            }
        }
        for (auto& th : t_vec) {
            th.join();
        }
        cout << "init with all pir servers\n";
        return pir_ips;
    }

    void query_pir_server_worker(string& queryStr, string& ip, uint64_t offset,
                                 bool recover) {
        int fd;
        SSL* ssl = ssl_connect_to_ip(ip, fd);
        assert(sendMsg(ssl, queryStr));
        string replyStr;
        assert(recvMsg(ssl, replyStr));
        if (!recover) {
            cout << "don't need to recover\n";
            return;
        }
        vector<uint8_t> reply = client.recoverReply(replyStr, offset);
        cout << "finish recover\n";
    }

    void generate_pir_queries(int elem_id, vector<uint64_t>& offsets,
                              vector<string>& queries) {
        queries = vector<string>(ips.size() * pir_server_num);
        offsets = vector<uint64_t>(ips.size() * pir_server_num);
        for (int i = 0; i < ips.size(); i++) {
            for (int j = 0; j < pir_server_num; j++) {
                int query_id = elem_id % (num_elem / pir_server_num);
                if (elem_id % pir_server_num != j) {
                    query_id = rand() % (num_elem / pir_server_num);
                }
                queries[i * pir_server_num + j] = client.generateQuery(
                    query_id, offsets[i * pir_server_num + j]);
            }
        }
    }

    void query_pir_servers(vector<string>& query_vec, vector<string>& pir_ips,
                           vector<uint64_t>& offsets, int idx) {
        vector<thread> t_vec;
        assert((ips.size() * pir_server_num) == pir_ips.size());
        for (int i = 0; i < ips.size(); i++) {
            for (int j = 0; j < pir_server_num; j++) {
                bool recover = true;
                if (idx % pir_server_num != j) {
                    recover = false;
                }
                thread t(&Client::query_pir_server_worker, this,
                         ref(query_vec[i * pir_server_num + j]),
                         ref(pir_ips[i * pir_server_num + j]),
                         offsets[i * pir_server_num + j], recover);
                t_vec.push_back(move(t));
            }
        }
        for (auto& th : t_vec) {
            th.join();
        }
        cout << "finish querying pir servers\n";
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
        vector<string> pir_ips = connect_pir_servers();

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

        // read from PIR servers
        system_clock::time_point start, end, s_, e_;
        start = system_clock::now();

        s_ = system_clock::now();
        query_pir_servers(query_vec, pir_ips, offsets, elem_idx);
        e_ = system_clock::now();
        double query_pir_time =
            duration_cast<std::chrono::duration<double>>(e_ - s_).count();
        cout << "pir time: " << query_pir_time << endl;

        s_ = system_clock::now();
        // randomize the shares
        for (int i = 0; i < bidder_num; i++) {
            vector<int> r;
            ae.randMask(r);
            vector<mpz_t> er = ae.encBid(r);
            encShare1[i] = ae.randomize(encShare1[i], er);
            encShare2[i] = ae.randomize(encShare2[i], er);
        }

        // TODO: REMOVE from here
        // submit to auction servers and receive response, all done!
        uint32_t winner = connect_auction_servers(encShare1, encShare2);
        e_ = system_clock::now();
        double auction_time =
            duration_cast<std::chrono::duration<double>>(e_ - s_).count();

        end = system_clock::now();

        cout << "winner bid: " << bids[winner] << endl;
        cout
            << "Total time: "
            << duration_cast<std::chrono::duration<double>>(end - start).count()
            << " sec" << endl;
        cout << "Time with cached bidding shares: " << auction_time << " sec"
             << endl;
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