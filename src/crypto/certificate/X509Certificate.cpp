#include "crypto/certificate/X509Certificate.h"
#include "crypto/opensslInterface/OpenSslInterface.h"
#include "io/Io.h"
#include <cstring>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <openssl/dh.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
#include <openssl/rand.h>
#include <openssl/ec.h>


using namespace Gicame::Crypto;



X509Certificate::X509Certificate(X509* cert) :
    ptr(cert, X509_free)
{
    // Nothing to do
}

std::vector<byte_t> X509Certificate::toDer() const {
    byte_t* buf = NULL;
    int len = i2d_X509(ptr.get(), &buf);

    if (unlikely(len < 0)) {
        throw RUNTIME_ERROR("i2d_X509 failed");
    }

    std::vector<byte_t> ret(len);
    memcpy(ret.data(), buf, len);
    OPENSSL_free(buf);
    return ret;
}

std::vector<byte_t> X509Certificate::toPem() const {
    BIO* bio = BIO_new(BIO_s_mem());
    if (unlikely(!bio)) {
        throw RUNTIME_ERROR("unable to allocate bio");
    }
    if (unlikely(!PEM_write_bio_X509(bio, ptr.get()))) {
        BIO_free(bio);
        throw RUNTIME_ERROR("unable to write certificate to bio");
    }
    std::vector<byte_t> ret = Gicame::Crypto::OpenSslInterface::bioToByteVector(bio);
    BIO_free(bio);
    return ret;
}

EvpKey X509Certificate::getPublicKey() const {
    EVP_PKEY* pubKey = X509_get_pubkey(ptr.get());
    if (unlikely(!pubKey)) {
        throw RUNTIME_ERROR("X509_get_pubkey failed. Malformed certificate?");
    }
    return EvpKey::fromOpenSslEvpKey(pubKey);
}

void X509Certificate::sign(EvpKey privKey) {
    if (unlikely(!X509_sign(ptr.get(), privKey.get(), EVP_sha1()))) {
        throw RUNTIME_ERROR("X509_sign failed");
    }
}

std::string X509Certificate::getSubjectName() const {
    X509_NAME* x509Name = X509_get_subject_name(ptr.get());
    if (unlikely(!x509Name)) {
        throw RUNTIME_ERROR("X509_get_subject_name failed. Malformed certificate?");
    }
    char* str = X509_NAME_oneline(x509Name, NULL, 0);
    if (unlikely(!str)) {
        throw RUNTIME_ERROR("X509_NAME_oneline failed");
    }
    std::string ret(str);
    OPENSSL_free(str);
    return ret;
}

X509Certificate& X509Certificate::setSubjectName(const std::string& field, const std::string& value) {
    X509_NAME* x509Name = X509_NAME_new();
    if (unlikely(!x509Name)) {
        throw RUNTIME_ERROR("X509_NAME_new failed");
    }

    if (unlikely(!X509_NAME_add_entry_by_txt(
        x509Name, field.c_str(), MBSTRING_ASC, (const unsigned char*)value.c_str(), -1, -1, 0)
    )) {
        throw RUNTIME_ERROR("X509_NAME_add_entry_by_txt failed");
    }

    if (unlikely(!X509_set_subject_name(ptr.get(), x509Name))) {
        throw RUNTIME_ERROR("X509_set_subject_name failed");
    }

    return *this;
}

X509Certificate& X509Certificate::addSubjectName(const std::string& field, const std::string& value) {
    X509_NAME* x509Name = X509_get_subject_name(ptr.get());
    if (unlikely(!x509Name)) {
        throw RUNTIME_ERROR("X509_get_subject_name failed");
    }

    if (unlikely(!X509_NAME_add_entry_by_txt(
        x509Name, field.c_str(), MBSTRING_ASC, (const unsigned char*)value.c_str(), -1, -1, 0)
    )) {
        throw RUNTIME_ERROR("X509_NAME_add_entry_by_txt failed");
    }

    if (unlikely(!X509_set_subject_name(ptr.get(), x509Name))) {
        throw RUNTIME_ERROR("X509_set_subject_name failed");
    }

    return *this;
}

std::string X509Certificate::getIssuerName() const {
    X509_NAME* x509Name = X509_get_issuer_name(ptr.get());
    if (unlikely(!x509Name)) {
        throw RUNTIME_ERROR("X509_get_issuer_name failed. Malformed certificate?");
    }
    char* str = X509_NAME_oneline(x509Name, NULL, 0);
    if (unlikely(!str)) {
        throw RUNTIME_ERROR("X509_NAME_oneline failed");
    }
    std::string ret(str);
    OPENSSL_free(str);
    return ret;
}

X509Certificate& X509Certificate::setIssuerName(const std::string& field, const std::string& value) {
    X509_NAME* x509Name = X509_NAME_new();
    if (unlikely(!x509Name)) {
        throw RUNTIME_ERROR("X509_NAME_new failed");
    }

    if (unlikely(!X509_NAME_add_entry_by_txt(
        x509Name, field.c_str(), MBSTRING_ASC, (const unsigned char*)value.c_str(), -1, -1, 0)
    )) {
        throw RUNTIME_ERROR("X509_NAME_add_entry_by_txt failed");
    }

    if (unlikely(!X509_set_issuer_name(ptr.get(), x509Name))) {
        throw RUNTIME_ERROR("X509_set_issuer_name failed");
    }

    return *this;
}

X509Certificate& X509Certificate::addIssuerName(const std::string& field, const std::string& value) {
    X509_NAME* x509Name = X509_get_issuer_name(ptr.get());
    if (unlikely(!x509Name)) {
        throw RUNTIME_ERROR("X509_get_issuer_name failed");
    }

    if (unlikely(!X509_NAME_add_entry_by_txt(
        x509Name, field.c_str(), MBSTRING_ASC, (const unsigned char*)value.c_str(), -1, -1, 0)
    )) {
        throw RUNTIME_ERROR("X509_NAME_add_entry_by_txt failed");
    }

    if (unlikely(!X509_set_issuer_name(ptr.get(), x509Name))) {
        throw RUNTIME_ERROR("X509_set_issuer_name failed");
    }

    return *this;
}

X509Certificate X509Certificate::newEmptyX509Certificate() {
    X509* cert = X509_new();
    if (unlikely(!cert)) {
        throw RUNTIME_ERROR("X509_new failed");
    }
    return X509Certificate(cert);
}

X509Certificate X509Certificate::fromDer(const std::vector<byte_t>& der) {
    const byte_t* derDataPtr = der.data();
    const size_t derSize = der.size();

    if (unlikely(derSize > LONG_MAX)) {
        throw RUNTIME_ERROR("derSize too big");
    }

    X509* cert = d2i_X509(NULL, (const byte_t**)&derDataPtr, (long)derSize);
    if (unlikely(!cert)) {
        throw RUNTIME_ERROR("d2i_X509 failed");
    }

    return X509Certificate(cert);
}

X509Certificate X509Certificate::fromPem(const byte_t* pem, const size_t size) {
    if (unlikely(size > (size_t)INT_MAX || size == 0u)) {
        throw RUNTIME_ERROR("invaldi size");
    }

    BIO* bio = BIO_new_mem_buf(pem, (int)size);

    if (unlikely(!bio)) {
        throw RUNTIME_ERROR("unable to allocate bio");
    }

    X509* cert = PEM_read_bio_X509(bio, NULL, NULL, NULL);
    BIO_free(bio);
    if (unlikely(!cert)) {
        throw RUNTIME_ERROR("unable to generate X509 from bio");
    }

    return X509Certificate(cert);
}

X509Certificate X509Certificate::fromPem(const std::vector<byte_t>& pem) {
    return X509Certificate::fromPem(pem.data(), pem.size());
}

X509Certificate X509Certificate::fromPem(const std::string& pem) {
    return X509Certificate::fromPem((byte_t*)pem.data(), pem.length() + 1u);
}


X509Certificate X509Certificate::fromDerFile(const std::string& filePath) {
    return X509Certificate::fromDer(Gicame::IO::readFileContent(filePath));
}

X509Certificate X509Certificate::fromPemFile(const std::string& filePath) {
    return X509Certificate::fromPem(Gicame::IO::readFileContent(filePath));
}
