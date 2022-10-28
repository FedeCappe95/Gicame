#include "crypto/ds/Signer.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>


using namespace Gicame::Crypto;


Signer::Signer() :
    ctx(NULL)
{
    digestAlg = EVP_sha256();
    ctx = EVP_MD_CTX_create();

    if (unlikely(!ctx)) {
        throw RUNTIME_ERROR("EVP_MD_CTX_create failed");
    }

    if (unlikely(!EVP_SignInit(ctx, digestAlg))) {
        EVP_MD_CTX_free(ctx);
        ctx = NULL;
        throw RUNTIME_ERROR("EVP_SignInit failed");
    }
}

Signer::~Signer() {
    if (likely(ctx))
        EVP_MD_CTX_free(ctx);
}

void Signer::update(const std::vector<byte_t>& source) {
    update(source.data(), source.size());
}

void Signer::update(const void* source, const size_t sourceSize) {
    if (unlikely(!EVP_SignUpdate(ctx, (byte_t*)source, sourceSize))) {
        throw RUNTIME_ERROR("EVP_SignUpdate failed");
    }
}

std::vector<byte_t> Signer::sign(EvpKey privKey) {
    unsigned int size = Gicame::Utilities::safeNumericCast<unsigned int>(privKey.maxOutputBufferSize());
    std::vector<byte_t> dest(size);
    if (unlikely(!EVP_SignFinal(ctx, dest.data(), &size, privKey.get()))) {
        throw RUNTIME_ERROR("EVP_SignFinal failed");
    }
    dest.resize(size);
    return dest;
}

size_t Signer::sign(EvpKey privKey, void* outSign) {
    unsigned int size;
    if (unlikely(!EVP_SignFinal(ctx, (byte_t*)outSign, &size, privKey.get()))) {
        throw RUNTIME_ERROR("EVP_SignFinal failed");
    }
    return Gicame::Utilities::safeNumericCast<size_t>(size);
}

std::vector<byte_t> Signer::staticSign(EvpKey privKey, const std::vector<byte_t>& source) {
    Signer signer;
    signer.update(source);
    return signer.sign(privKey);
}

std::vector<byte_t> Signer::staticSign(EvpKey privKey, const void* source, const size_t sourceSize) {
    Signer signer;
    signer.update(source, sourceSize);
    return signer.sign(privKey);
}

size_t Signer::staticSign(EvpKey privKey, const void* source, const size_t sourceSize, void* dest) {
    Signer signer;
    signer.update(source, sourceSize);
    return signer.sign(privKey, dest);
}
