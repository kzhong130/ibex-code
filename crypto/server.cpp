#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/dh.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>

#include "lwe.cpp"

class SignedEncShare {
   public:
    Ciphertext c;
    string signature;
    string c_msg;

    size_t size() { return signature.size() + c_msg.size(); }
};

// class contains operations for both aux server and main server
class Server {
   private:
    RSA* privateRSA;
    RSA* publicRSA;

    Plaintext genPlaintextWithId(int id) {
        assert(id < lwe.getDegree() / 2);
        Plaintext p;
        p.m = vector<mpz_t>(lwe.getDegree());
        for (int i = 0; i < p.m.size(); i++) {
            mpz_init(p.m[i]);
        }
        mpz_set_si(p.m[id], 1);
        return p;
    }

    // examples from
    // https://gist.github.com/irbull/08339ddcd5686f509e9826964b17bb59
    bool RSASign(RSA* rsa, const unsigned char* Msg, size_t MsgLen,
                 unsigned char** EncMsg, size_t* MsgLenEnc) {
        EVP_MD_CTX* m_RSASignCtx = EVP_MD_CTX_create();
        EVP_PKEY* priKey = EVP_PKEY_new();
        EVP_PKEY_assign_RSA(priKey, rsa);
        if (EVP_DigestSignInit(m_RSASignCtx, NULL, EVP_sha256(), NULL,
                               priKey) <= 0) {
            return false;
        }
        if (EVP_DigestSignUpdate(m_RSASignCtx, Msg, MsgLen) <= 0) {
            return false;
        }
        if (EVP_DigestSignFinal(m_RSASignCtx, NULL, MsgLenEnc) <= 0) {
            return false;
        }
        *EncMsg = (unsigned char*)malloc(*MsgLenEnc);
        if (EVP_DigestSignFinal(m_RSASignCtx, *EncMsg, MsgLenEnc) <= 0) {
            return false;
        }
        return true;
    }

    bool RSAVerifySignature(RSA* rsa, unsigned char* MsgHash, size_t MsgHashLen,
                            const char* Msg, size_t MsgLen, bool* Authentic) {
        *Authentic = false;
        EVP_PKEY* pubKey = EVP_PKEY_new();
        EVP_PKEY_assign_RSA(pubKey, rsa);
        EVP_MD_CTX* m_RSAVerifyCtx = EVP_MD_CTX_create();

        if (EVP_DigestVerifyInit(m_RSAVerifyCtx, NULL, EVP_sha256(), NULL,
                                 pubKey) <= 0) {
            return false;
        }
        if (EVP_DigestVerifyUpdate(m_RSAVerifyCtx, Msg, MsgLen) <= 0) {
            return false;
        }
        int AuthStatus =
            EVP_DigestVerifyFinal(m_RSAVerifyCtx, MsgHash, MsgHashLen);
        if (AuthStatus == 1) {
            *Authentic = true;
            return true;
        } else if (AuthStatus == 0) {
            *Authentic = false;
            return true;
        } else {
            *Authentic = false;
            return false;
        }
    }

    unsigned char* signMessage(std::string plainText, size_t* signatureLength) {
        unsigned char* signature;
        RSASign(privateRSA, (unsigned char*)plainText.c_str(),
                plainText.length(), &signature, signatureLength);
        return signature;
    }

    bool verifySignature(std::string plainText, unsigned char* signature,
                         size_t signatureLength) {
        bool authentic;
        bool result = RSAVerifySignature(publicRSA, signature, signatureLength,
                                         plainText.c_str(), plainText.length(),
                                         &authentic);
        return result & authentic;
    }

   public:
    LWE lwe;

    Server(string pStr, string wStr, int nbits, string pd, int logP, bool ks) {
        lwe = LWE(pStr, wStr, nbits, pd, logP, ks);
        publicRSA = RSA_new();
        privateRSA = RSA_new();

        FILE* PubKeyFile = NULL;
        FILE* PrivKeyFile = NULL;
        if ((PrivKeyFile = fopen("./rsa-keys/private-key", "rb")) == NULL) {
            perror("open");
            assert(0);
        }
        if (PEM_read_RSAPrivateKey(PrivKeyFile, &privateRSA, NULL, NULL) ==
            NULL) {
            assert(0);
        }
        if ((PubKeyFile = fopen("./rsa-keys/public-key", "rb")) == NULL) {
            assert(0);
        }
        if (PEM_read_RSA_PUBKEY(PubKeyFile, &publicRSA, 0, 0) == NULL) {
            assert(0);
        }
        fclose(PubKeyFile);
        fclose(PrivKeyFile);
    }

    SignedEncShare encShare(int s) {
        SignedEncShare se;
        Plaintext p = genPlaintextWithId(s);
        se.c = lwe.enc(p);

        // generate signature
        se.c_msg = se.c.serialize();
        size_t sig_size;
        unsigned char* sig = signMessage(se.c_msg, &sig_size);
        se.signature = string((const char*)sig, sig_size);
        return se;
    }

    Ciphertext recoverReport(int s, SignedEncShare& auxShare) {
        Plaintext p = genPlaintextWithId(s);
        Ciphertext c = lwe.enc(p);
        Ciphertext r = lwe.mul(c, auxShare.c);

        // verify signature
        assert(verifySignature(auxShare.c_msg,
                               (unsigned char*)auxShare.signature.c_str(),
                               auxShare.signature.size()));
        p.free();
        c.free();
        return r;
    }

    Ciphertext aggregateReports(Ciphertext& c1, Ciphertext& c2) {
        return lwe.add(c1, c2);
    }
};

// Helper class of a client instance
class Client {
   private:
    int range;

   public:
    Client(int nbits) {
        srand(time(0));
        this->range = (1 << nbits) / 2;
    }

    int randShare() { return rand() % (this->range); }
};
