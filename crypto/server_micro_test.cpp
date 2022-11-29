#include "server.cpp"

int main(int argc, char* argv[]) {
    int nbits = 16;
    string pd_str = "134217728";
    int LOGP = 108;
    bool ks = false;
    string pStr = "558127740940706268294329795608577";
    string wStr = "3";
    int times = 10;

    int opt;
    while ((opt = getopt(argc, argv, "b:d:l:p:w:t:")) != -1) {
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
        }
    }

    Server server(pStr, wStr, nbits, pd_str, LOGP, ks);
    Client client(nbits);

    srand(time(0));
    int testId = rand() % times;

    // counter of time measurement
    double enc_time = 0.0;
    double agg_time = 0.0;
    double recover_time = 0.0;
    double generate_share_time = 0.0;
    size_t avg_size_enc_share = 0;
    size_t avg_size_enc_report = 0;

    system_clock::time_point start, end;
    for (int i = 0; i < times; i++) {
        // client generate two shares
        start = system_clock::now();
        int s1 = client.randShare();
        int s2 = client.randShare();
        end = system_clock::now();
        generate_share_time +=
            duration_cast<std::chrono::duration<double>>(end - start).count();

        // Test aux server generate encrypted share
        start = system_clock::now();
        SignedEncShare enc_s1 = server.encShare(s1);
        end = system_clock::now();
        enc_time +=
            duration_cast<std::chrono::duration<double>>(end - start).count();
        cout
            << "aux server generates encrypted share " << i << " times: "
            << duration_cast<std::chrono::duration<double>>(end - start).count()
            << endl;

        avg_size_enc_share += enc_s1.size();

        // Test main server recover the report
        start = system_clock::now();
        Ciphertext enc_report = server.recoverReport(s2, enc_s1);
        end = system_clock::now();
        recover_time +=
            duration_cast<std::chrono::duration<double>>(end - start).count();
        cout
            << "main server recovers encrypted report " << i << " times: "
            << duration_cast<std::chrono::duration<double>>(end - start).count()
            << endl;

        avg_size_enc_report += enc_report.serialize().size();

        // Test correctness of encrypted report
        if (i == testId) {
            Plaintext dec_res = server.lwe.dec(enc_report);
            for (int i = 0; i < dec_res.m.size(); i++) {
                if (i == (s1 + s2)) {
                    assert(mpz_cmp_si(dec_res.m[i], 1) == 0);
                } else {
                    assert(mpz_cmp_si(dec_res.m[i], 0) == 0);
                }
            }
            cout << "randomly test " << testId
                 << " for correctness, test passed\n";
        }

        // Test one addition of encrypted report
        start = system_clock::now();
        Ciphertext add_report = server.aggregateReports(enc_report, enc_report);
        end = system_clock::now();
        agg_time +=
            duration_cast<std::chrono::duration<double>>(end - start).count();
        cout
            << "main server aggregate encrypted reports " << i << " times: "
            << duration_cast<std::chrono::duration<double>>(end - start).count()
            << endl;
    }
    cout << "(avg) browser generate share: " << generate_share_time / times
         << endl;
    cout << "(avg) aux server encrypts share: " << enc_time / times << endl;
    cout << "(avg) main server recovers report: " << recover_time / times
         << endl;
    cout << "(avg) main server aggregates reports: " << agg_time / times
         << endl;
    cout << "(avg) size of encrypted share by aux server: "
         << avg_size_enc_share / times << " (bytes)" << endl;
    cout << "(avg) size of encrypted report: " << avg_size_enc_report / times
         << " (bytes)" << endl;

    return 0;
}