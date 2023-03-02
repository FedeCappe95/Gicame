#include "crypto/encryption/Encryptor.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <algorithm>


using namespace Gicame::Crypto;


// Macro
#define ecctx ((EVP_CIPHER_CTX*)(ctx))


Encryptor::Encryptor(const SymmetricKey& key, const EncryptionAlgorithm algorithm, const byte_t* iv) :
	key(key),
	ctx(NULL),
	ciphertextLen(0),
	algorithm(algorithm)
{
	if (unlikely(algorithm.keySize() != key.getKeySize())) {
		throw RUNTIME_ERROR("key length is not correct");
	}

	ctx = EVP_CIPHER_CTX_new();
	if (unlikely(!ecctx)) {
		throw RUNTIME_ERROR("EVP_CIPHER_CTX_new() failed");
	}

	if (unlikely(!EVP_EncryptInit(
		ecctx, algorithm.toEvpCipher(), this->key.get(), iv
	))) {
		EVP_CIPHER_CTX_free(ecctx);
		ctx = NULL;
		throw RUNTIME_ERROR("EVP_EncryptInit() failed");
	}
}

Encryptor::~Encryptor() {
    if (likely(ecctx)) {
		EVP_CIPHER_CTX_free(ecctx);
	}
}

size_t Encryptor::update(const void* source, const size_t sourceSize, void* out) {
	constexpr size_t MAX_SHOT_SIZE = (size_t)INT_MAX;

	if (unlikely(!ecctx)) {
		throw RUNTIME_ERROR("ctx is NULL. finalized() already called?");
	}

	size_t remainingSize = sourceSize;
	const byte_t* sourcePtr = (byte_t*)source;
	byte_t* outPtr = (byte_t*)out;
	size_t shotLen = 0;
	while (remainingSize) {
		const int inLen = (int)std::min(MAX_SHOT_SIZE, remainingSize);  // Cast is safe since MAX_SHOT_SIZE <= INT_MAX
		int outLen = inLen;
		if (unlikely(!EVP_EncryptUpdate(ecctx, outPtr, &outLen, sourcePtr, inLen))) {
			throw RUNTIME_ERROR("EVP_EncryptUpdate failed");
		}
		remainingSize -= (size_t)inLen;
		shotLen += (size_t)outLen;
		sourcePtr += (size_t)inLen;
		outPtr += (size_t)inLen;
	}

	ciphertextLen += (size_t)shotLen;

	return shotLen;
}

std::vector<byte_t> Encryptor::update(const void* source, const size_t sourceSize) {
	// sourceSize is the max size of the chipertext (this block)
	// shotLen will be the real size of the chipertext (this block)
	std::vector<byte_t> ciphertext(sourceSize);
	const size_t shotLen = update(source, sourceSize, ciphertext.data());
	ciphertext.resize(shotLen);
	return ciphertext;
}

size_t Encryptor::finalize(void* out) {
	if (unlikely(!ecctx)) {
		throw RUNTIME_ERROR("ctx is NULL. finalized() already called?");
	}

	int paddingLen;

	if (unlikely(!EVP_EncryptFinal(ecctx, (byte_t*)out, &paddingLen))) {
		throw RUNTIME_ERROR("EVP_EncryptFinal failed");
	}

	ciphertextLen += paddingLen;

	EVP_CIPHER_CTX_free(ecctx);
	ctx = NULL;

	return (size_t)paddingLen;
}

std::vector<byte_t> Encryptor::finalize() {
    std::vector<byte_t> ciphertext(algorithm.blockSize());

	const size_t paddingLen = finalize(ciphertext.data());

    if (unlikely(ciphertext.size() > paddingLen)) {
		ciphertext.resize(paddingLen);
	}

    return ciphertext;
}

size_t Encryptor::getCiphertextLen() {
    return ciphertextLen;
}

size_t Encryptor::encrypt(const SymmetricKey& key, const EncryptionAlgorithm algorithm, const byte_t* iv, const void* source, const size_t sourceSize, void* dest) {
	Encryptor encryptor(key, algorithm, iv);
	const size_t shotLen = encryptor.update(source, sourceSize, dest);
	const size_t paddingLen = encryptor.finalize((byte_t*)dest + shotLen);
	return shotLen + paddingLen;
}

std::vector<byte_t> Encryptor::encrypt(const SymmetricKey& key, const EncryptionAlgorithm algorithm, const byte_t* iv, const void* source, const size_t sourceSize) {
	Encryptor encryptor(key, algorithm, iv);
	std::vector<byte_t> result = encryptor.update(source, sourceSize);
	const std::vector<byte_t> resultFinal = encryptor.finalize();
	result.insert(result.end(), resultFinal.begin(), resultFinal.end());
	return result;
}
