#include "aggregator.hpp"

int main(int argc, char* argv[]) {
    int opt;
    int log_degree = 15;
    int times = 10;
    while ((opt = getopt(argc, argv, "d:t:")) != -1) {
        if (opt == 'd') {
            log_degree = atoi(optarg);
        } else if (opt == 't') {
            times = atoi(optarg);
        }
    }

    // counter of time measurement
    double enc_time = 0.0;
    double agg_time = 0.0;
    double recover_time = 0.0;
    double generate_share_time = 0.0;
    double dec_time = 0.0;
    size_t avg_size_enc_share = 0;
    size_t avg_size_enc_report = 0;

    system_clock::time_point start, end;

    Aggregator agg(log_degree);
    Client client(log_degree);

    for (int i = 0; i < times; i++) {
        int group_ = client.randReport();
        start = system_clock::now();
        int s1 = client.randShare();
        int s2 = (group_ - s1);
        if (s2 < 0) s2 += (1 << log_degree);
        end = system_clock::now();
        generate_share_time +=
            duration_cast<std::chrono::duration<double>>(end - start).count();

        start = system_clock::now();
        SignedEncShare c = agg.encShare(s1);
        end = system_clock::now();
        enc_time +=
            duration_cast<std::chrono::duration<double>>(end - start).count();

        avg_size_enc_share += c.size();

        start = system_clock::now();
        Ciphertext report;
        assert(agg.recoverReport(c, s2, report));
        end = system_clock::now();
        recover_time +=
            duration_cast<std::chrono::duration<double>>(end - start).count();

        avg_size_enc_report += agg.cipher_size(report);

        start = system_clock::now();
        Ciphertext aggRes = agg.aggregate(report, report);
        end = system_clock::now();
        agg_time +=
            duration_cast<std::chrono::duration<double>>(end - start).count();

        start = system_clock::now();
        Plaintext decRes = agg.dec(aggRes);
        end = system_clock::now();
        dec_time +=
            duration_cast<std::chrono::duration<double>>(end - start).count();

        // check the decRes
        string poly = "2x^";
        poly.append(to_string(s1 + s2));
        assert(s1 + s2 < (1 << (log_degree + 1)));
        Plaintext expected = Plaintext(poly);
        assert(decRes == expected);
    }

    cout << "(avg) browser generate share (ms): "
         << generate_share_time / times * 1000.0 << endl;
    cout << "(avg) aux server encrypts share (ms): "
         << enc_time / times * 1000.0 << endl;
    cout << "(avg) aux server decrypts aggregation result (ms): "
         << dec_time / times * 1000.0 << endl;
    cout << "(avg) main server recovers report (ms): "
         << recover_time / times * 1000.0 << endl;
    cout << "(avg) main server aggregates reports (ms): "
         << agg_time / times * 1000.0 << endl;
    cout << "(avg) size of encrypted share by aux server: "
         << avg_size_enc_share / times / 1024.0 / 1024.0 << " (MB)" << endl;
    cout << "(avg) size of encrypted report: "
         << avg_size_enc_report / times / 1024.0 / 1024.0 << " (MB)" << endl;
    return 0;
}