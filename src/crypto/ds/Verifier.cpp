#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include "crypto/ds/Verifier.h"


using namespace Gicame::Crypto;


Verifier::Verifier() {
    digestAlg = EVP_sha256();
    ctx = EVP_MD_CTX_create();

    if (unlikely(!ctx)) {
        throw RUNTIME_ERROR("EVP_MD_CTX_create failed");
    }

    if (unlikely(!EVP_VerifyInit(ctx, digestAlg))) {
        EVP_MD_CTX_destroy(ctx);
        ctx = NULL;
        throw RUNTIME_ERROR("EVP_VerifyInit failed");
    }
}

Verifier::~Verifier() {
    if (likely(ctx)) {
        EVP_MD_CTX_destroy(ctx);
    }
}


void Verifier::update(const std::vector<byte_t>& source) {
    update(source.data(), source.size());
}

void Verifier::update(const void* source, const size_t size) {
    if (unlikely(!EVP_VerifyUpdate(ctx, (byte_t*)source, size))) {
        throw RUNTIME_ERROR("EVP_VerifyUpdate failed");
    }
}

bool Verifier::finalize(EvpKey pubKey, const std::vector<byte_t>& sign) {
    return finalize(pubKey, sign.data(), sign.size());
}

bool Verifier::finalize(EvpKey pubKey, const void* sign, const size_t size) {
    const unsigned int sizeAsUi = Gicame::Utilities::safeNumericCast<unsigned int>(size);
    return EVP_VerifyFinal(ctx, (byte_t*)sign, sizeAsUi, pubKey.get()) == 1;
}

bool Verifier::verify(
    EvpKey pubKey, const std::vector<byte_t>& source, const std::vector<byte_t>& sign
) {
    Verifier verifier;
    verifier.update(source);
    return verifier.finalize(pubKey, sign);
}

bool Verifier::verify(
    EvpKey pubKey, const void* source, const size_t sourceSize, const void* sign,
    const size_t signSize
) {
    Verifier verifier;
    verifier.update(source, sourceSize);
    return verifier.finalize(pubKey, sign, signSize);
}
