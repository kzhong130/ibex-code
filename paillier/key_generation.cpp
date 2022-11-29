#include <chrono>
#include <iostream>

#include "paillier.c"
#include "tools.c"

using namespace std;
using namespace chrono;

int main() {
    system_clock::time_point starttime, endtime;

    // Initialize
    paillier_public_key* pub_key = new paillier_public_key();
    paillier_private_key* priv_key = new paillier_private_key();
    paillier_public_init(pub_key);
    paillier_private_init(priv_key);
    mp_bitcnt_t length = 2048;

    // Key generation
    starttime = system_clock::now();
    paillier_keygen(pub_key, priv_key, length);
    endtime = system_clock::now();
    cout << "Key Generation time: "
         << duration_cast<duration<double>>(endtime - starttime).count()
         << endl;

    // Key Export
    FILE* pub_key_file = fopen("public.key", "wb");
    paillier_public_out_str(pub_key_file, pub_key);
    fclose(pub_key_file);
    FILE* priv_key_file = fopen("private.key", "wb");
    paillier_private_out_str(priv_key_file, priv_key);
    fclose(priv_key_file);

    paillier_public_clear(pub_key);
    paillier_private_clear(priv_key);

    return 0;
}