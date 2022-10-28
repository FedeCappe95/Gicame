#include "crypto/hash/HashCalculator.h"
#include <openssl/evp.h>


using namespace Gicame::Crypto;


std::vector<byte_t> HashCalculator::hash(const HashAlgorithm hashAlgorithm, const byte_t* source, const size_t sourceSize) {
	std::vector<byte_t> digest(hashAlgorithm.hashSize());
	HashCalculator::hash(hashAlgorithm, source, sourceSize, digest.data());
	return digest;
}

size_t HashCalculator::hash(const HashAlgorithm hashAlgorithm, const byte_t* source, const size_t sourceSize, void* out) {
	EVP_MD_CTX *mdctx = EVP_MD_CTX_create();

	if (unlikely(!mdctx)) {
		throw RUNTIME_ERROR("failed EVP_MD_CTX_create()");
	}

	if (unlikely(!EVP_DigestInit_ex(mdctx, hashAlgorithm.toEvpMd(), NULL))) {
		EVP_MD_CTX_destroy(mdctx);
		throw RUNTIME_ERROR("failed EVP_DigestInit_ex()");
	}

	if (unlikely(!EVP_DigestUpdate(mdctx, source, sourceSize))) {
		EVP_MD_CTX_destroy(mdctx);
		throw RUNTIME_ERROR("failed EVP_DigestUpdate()");
	}

	unsigned int digest_len;

	if (unlikely(!EVP_DigestFinal_ex(mdctx, (byte_t*)out, &digest_len))) {
		EVP_MD_CTX_destroy(mdctx);
		throw RUNTIME_ERROR("failed EVP_DigestFinal_ex()");
	}

	EVP_MD_CTX_destroy(mdctx);

	return digest_len;
}
