#include "crypto/key/EvpKey.h"
#include "crypto/opensslInterface/OpenSslInterface.h"
#include "io/Io.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/dh.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
#include <openssl/rand.h>
#include <openssl/ec.h>


using namespace Gicame::Crypto;


EvpKey::EvpKey(EVP_PKEY* evpKeyLegacy) :
    sp(evpKeyLegacy, EVP_PKEY_free)
{
    // Do nothing
}

EVP_PKEY* EvpKey::get() {
    return sp.get();
}

std::vector<byte_t> EvpKey::publicKeyToPem() const {
    BIO* bio = BIO_new(BIO_s_mem());
    if (unlikely(!bio)) {
        throw RUNTIME_ERROR("unable to allocate bio");
    }
    if (unlikely(!PEM_write_bio_PUBKEY(bio, sp.get()))) {
        BIO_free(bio);
        throw RUNTIME_ERROR("unable to write key");
    }
    std::vector<byte_t> ret = Gicame::Crypto::OpenSslInterface::bioToByteVector(bio);
    BIO_free(bio);
    return ret;
}

std::vector<byte_t> EvpKey::privateKeyToPem() const {
    BIO* bio = BIO_new(BIO_s_mem());
    if (unlikely(!bio)) {
        throw RUNTIME_ERROR("unable to allocate bio");
    }
    if (unlikely(!PEM_write_bio_PrivateKey(bio, sp.get(), NULL, NULL, 0, 0, NULL))) {
        BIO_free(bio);
        throw RUNTIME_ERROR("unable to write key");
    }
    std::vector<byte_t> ret = Gicame::Crypto::OpenSslInterface::bioToByteVector(bio);
    BIO_free(bio);
    return ret;
}

void EvpKey::publicKeySaveToPemFile(const std::string& path) const {
    FILE* keyFile;
    fopen_s(&keyFile, path.c_str(), "w");
    if (unlikely(!keyFile)) {
        throw RUNTIME_ERROR("unable to open key file");
    }
    if (unlikely(!PEM_write_PUBKEY(keyFile, sp.get()))) {
        fclose(keyFile);
        throw RUNTIME_ERROR("unable to write key file");
    }
    fclose(keyFile);
}

void EvpKey::privateKeySaveToPemFile(const std::string& path) const {
    FILE* keyFile;
    fopen_s(&keyFile, path.c_str(), "w");
    if (unlikely(!keyFile)) {
        throw RUNTIME_ERROR("unable to open key file");
    }
    if (unlikely(!PEM_write_PrivateKey(keyFile, sp.get(), NULL, NULL, 0, 0, NULL))) {
        fclose(keyFile);
        throw RUNTIME_ERROR("unable to write key file");
    }
    fclose(keyFile);
}

size_t EvpKey::maxOutputBufferSize() const {
    return (size_t)EVP_PKEY_size(sp.get());
}

EvpKey EvpKey::fromOpenSslEvpKey(EVP_PKEY* evpKeyLegacy) {
    if (unlikely(!evpKeyLegacy)) {
        throw RUNTIME_ERROR("evpKeyLegacy is NULL");
    }
    return EvpKey(evpKeyLegacy);
}

EvpKey EvpKey::newEmptyEvpKey() {
    return EvpKey::fromOpenSslEvpKey(EVP_PKEY_new());
}

EvpKey EvpKey::fromPublicKeyPem(const std::vector<byte_t>& data) {
	if (unlikely(data.size() > (size_t)INT_MAX)) {
		throw RUNTIME_ERROR("serialized too big");
	}

    BIO* bio = BIO_new_mem_buf(data.data(), (int)data.size());
    if (unlikely(!bio)) {
        throw RUNTIME_ERROR("unable to allocate bio");
    }
    EVP_PKEY* evp = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
    BIO_free(bio);
    if (unlikely(!evp)) {
        throw RUNTIME_ERROR("unable to generate EVP_PKEY from bio");
    }

    return EvpKey::fromOpenSslEvpKey(evp);
}

EvpKey EvpKey::fromPrivateKeyPem(const std::vector<byte_t>& data) {
	if (unlikely(data.size() > (size_t)INT_MAX)) {
		throw RUNTIME_ERROR("serialized too big");
	}

    BIO* bio = BIO_new_mem_buf(data.data(), (int)data.size());
    if (unlikely(!bio)) {
        throw RUNTIME_ERROR("unable to allocate bio");
    }
    EVP_PKEY* evp = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
    BIO_free(bio);
    if (unlikely(!evp)) {
        throw RUNTIME_ERROR("unable to generate EVP_PKEY from bio");
    }

    return EvpKey::fromOpenSslEvpKey(evp);
}

EvpKey EvpKey::fromPublicKeyPemFile(const std::string& path) {
    return EvpKey::fromPublicKeyPem(Gicame::IO::readFileContent(path));
}

EvpKey EvpKey::fromPrivateKeyPemFile(const std::string& path) {
    return EvpKey::fromPrivateKeyPem(Gicame::IO::readFileContent(path));
}
