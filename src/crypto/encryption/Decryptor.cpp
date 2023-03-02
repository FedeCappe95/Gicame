#include "crypto/encryption/Decryptor.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <algorithm>


using namespace Gicame::Crypto;


// Macro
#define ecctx ((EVP_CIPHER_CTX*)(ctx))


Decryptor::Decryptor(const SymmetricKey& key, const EncryptionAlgorithm algorithm, const byte_t* iv) :
    key(key),
    ctx(NULL),
    plaintextLen(0),
    algorithm(algorithm)
{
    if (unlikely(algorithm.keySize() != key.getKeySize())) {
		throw RUNTIME_ERROR("key length is not correct");
	}

    EVP_CIPHER_CTX* cc = EVP_CIPHER_CTX_new();
    ctx = cc;
	if (unlikely(!ctx)) {
        throw RUNTIME_ERROR("EVP_CIPHER_CTX_new() failed");
	}

    if (unlikely(!EVP_DecryptInit(
        ecctx, algorithm.toEvpCipher(), this->key.get(), iv
    ))) {
        EVP_CIPHER_CTX_free(ecctx);
		ctx = NULL;
		throw RUNTIME_ERROR("EVP_DecryptInit() failed");
    }
}

Decryptor::~Decryptor() {
    if (likely(ecctx)) {
		EVP_CIPHER_CTX_free(ecctx);
	}
}

size_t Decryptor::update(const void* source, const size_t sourceSize, void* out) {
    if (unlikely(!ecctx)) {
        throw RUNTIME_ERROR("ctx is NULL. finalized() already called?");
    }
    
    constexpr size_t MAX_SHOT_SIZE = (size_t)INT_MAX;

    size_t remainingSize = sourceSize;
    const byte_t* sourcePtr = (byte_t*)source;
    byte_t* outPtr = (byte_t*)out;
    size_t shotLen = 0;
    while (remainingSize) {
        const int inLen = (int)std::min(MAX_SHOT_SIZE, remainingSize);  // Cast is safe since MAX_SHOT_SIZE <= INT_MAX
        int outLen = inLen;
        if (unlikely(!EVP_DecryptUpdate(ecctx, outPtr, &outLen, sourcePtr, inLen))) {
            throw RUNTIME_ERROR("EVP_DecryptUpdate failed");
        }
        remainingSize -= (size_t)inLen;
        shotLen += (size_t)outLen;
        sourcePtr += (size_t)inLen;
        outPtr += (size_t)inLen;
    }

    plaintextLen += (size_t)shotLen;

    return shotLen;
}

std::vector<byte_t> Decryptor::update(const void* source, const size_t sourceSize) {
	// sourceSize + 16 is the max size of the plaintext (this block)
	// len will be the real size of the plaintext (this block)
    std::vector<byte_t> plaintext(sourceSize + 16);
    const size_t len = update(source, sourceSize, plaintext.data());

    if (unlikely(len < plaintext.size())) {
        plaintext.resize(len);
    }

    return plaintext;
}

size_t Decryptor::finalize(void* out) {
    if (unlikely(!ecctx)) {
        throw RUNTIME_ERROR("ctx is NULL. finalized() already called?");
    }

    int lastLen;

    if (unlikely(!EVP_DecryptFinal(ecctx, (byte_t*)out, &lastLen))) {
        throw RUNTIME_ERROR("EVP_DecryptFinal() failed");
    }

    plaintextLen += (size_t)lastLen;

    EVP_CIPHER_CTX_free(ecctx);
    ctx = NULL;

    return lastLen;
}

std::vector<byte_t> Decryptor::finalize() {
    std::vector<byte_t> plaintext(algorithm.blockSize());

    size_t lastLen = finalize(plaintext.data());

    if (unlikely(plaintext.size() > lastLen)) {
        plaintext.resize(lastLen);
    }

    return plaintext;
}

size_t Decryptor::getPlaintextLen() {
    return plaintextLen;
}

size_t Decryptor::decrypt(SymmetricKey key, const EncryptionAlgorithm algorithm, const byte_t* iv, const void* source, const size_t sourceSize, void* dest) {
    Decryptor decryptor(key, algorithm, iv);
    const size_t len = decryptor.update(source, sourceSize, dest);
    const size_t lastLen = decryptor.finalize((byte_t*)dest + len);
    return len + lastLen;
}

std::vector<byte_t> Decryptor::decrypt(SymmetricKey key, const EncryptionAlgorithm algorithm, const byte_t* iv, const void* source, const size_t sourceSize) {
    std::vector<byte_t> dest(sourceSize);
    size_t len = Decryptor::decrypt(key, algorithm, iv, source, sourceSize, dest.data());
    dest.resize(len);
    return dest;
}
