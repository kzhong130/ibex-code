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
#include <stdlib.h>
#include <unistd.h>
#include <random>

#include "he.hpp"

class SignedEncShare {
   public:
    Ciphertext c;
    string signature;
    string c_msg;

    size_t size() { return signature.size() + c_msg.size(); }
};

class Aggregator {
   private:
    HE he;

    RSA* privateRSA;
    RSA* publicRSA;

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

    string signMessage(string plainText) {
        unsigned char* signature;
        size_t signatureLength;
        RSASign(privateRSA, (unsigned char*)plainText.c_str(),
                plainText.length(), &signature, &signatureLength);
        return string((const char*)signature, signatureLength);
        ;
    }

    bool verifySignature(string plainText, string signature) {
        bool authentic;
        bool result = RSAVerifySignature(
            publicRSA, (unsigned char*)signature.c_str(), signature.size(),
            plainText.c_str(), plainText.length(), &authentic);
        return result & authentic;
    }

   public:
    Aggregator(int log_group, int plain_modulus_ = (1 << 27)) {
        he = HE(log_group, plain_modulus_);
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

    SignedEncShare encShare(int id) {
        SignedEncShare cs;
        he.encShare(id, cs.c);
        cs.c_msg = he.serializeCiphertext(cs.c);
        cs.signature = signMessage(cs.c_msg);
        return cs;
    }

    bool recoverReport(SignedEncShare& cs, int s, Ciphertext& report) {
        if (!verifySignature(cs.c_msg, cs.signature)) return false;
        he.shift(cs.c, s, report);
        return true;
    }

    void aggLimit() {
        cout << "agg limit is 2^{" << he.aggLimit() << "}" << endl;
    }

    Plaintext dec(Ciphertext& c) {
        Plaintext p;
        he.dec(c, p);
        return p;
    }

    size_t cipher_size(Ciphertext& c) {
        return he.serializeCiphertext(c).size();
    }

    Ciphertext aggregate(Ciphertext& c1, Ciphertext& c2) {
        Ciphertext c12;
        he.agg(c1, c2, c12);
        return c12;
    }
};

// Helper class of a client instance
class Client {
   private:
    int range;
    random_device rd;

   public:
    Client(int log_group) {
        srand(time(0));
        // 1<<(log_group+1) is polynomial degree
        this->range = (1 << log_group);
    }

    int randReport() {return rand() % (this->range);}

    inline int randShare() { return rd() % (this->range); }
};
