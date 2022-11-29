#include <thread>

#include "server.cpp"

void agg(int times, int nbits, string pd_str, int LOGP, bool ks, string pStr,
         string wStr) {
    Server server(pStr, wStr, nbits, pd_str, LOGP, ks);
    Client client(nbits);
    int s1 = client.randShare();
    int s2 = client.randShare();

    // Test aux server generate encrypted share
    SignedEncShare enc_s1 = server.encShare(s1);

    for (int i = 0; i < times; i++) {
        Ciphertext enc_report = server.recoverReport(s2, enc_s1);
        Ciphertext add_report = server.aggregateReports(enc_report, enc_report);
        enc_report.free();
        add_report.free();
    }
    return;
}

int main(int argc, char* argv[]) {
    int nbits = 16;
    string pd_str = "134217728";
    int LOGP = 108;
    bool ks = false;
    string pStr = "558127740940706268294329795608577";
    string wStr = "3";
    int times = 1;
    int parallel_num = 8;

    int opt;
    while ((opt = getopt(argc, argv, "b:d:l:p:w:t:n:")) != -1) {
        if (opt == 'b') {
            nbits = atoi(optarg);
        } else if (opt == 'd') {
            pd_str = string(optarg);
        } else if (opt == 'l') {
            LOGP = atoi(optarg);
        } else if (opt == 'p') {
            pStr = string(optarg);
        } else if (opt == 'w') {
            wStr = string(optarg);
        } else if (opt == 't') {
            times = atoi(optarg);
        } else if (opt == 'n') {
            parallel_num = atoi(optarg);
        }
    }
    system_clock::time_point starttime, endtime;
    srand(time(0));
    vector<thread> t_vec;
    starttime = system_clock::now();

    for (int i = 0; i < parallel_num; i++) {
        thread t(&agg, times, nbits, pd_str, LOGP, ks, pStr, wStr);
        t_vec.push_back(move(t));
    }
    for (auto& th : t_vec) {
        th.join();
    }
    endtime = system_clock::now();
    cout << "agg time of " << parallel_num * times << " tasks: "
         << duration_cast<duration<double>>(endtime - starttime).count()
         << endl;

    return 0;
}