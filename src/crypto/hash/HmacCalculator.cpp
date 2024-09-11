#include "crypto/hash/HmacCalculator.h"
#include "utils/Numbers.h"
#include <openssl/conf.h>
#include <openssl/hmac.h>


using namespace Gicame::Crypto;


HmacCalculator::HmacCalculator(const SymmetricKey& key, const HashAlgorithm hashAlgorithm) :
	key(key),
	hashAlgorithm(hashAlgorithm)
{
	if (unlikely(this->hashAlgorithm.keySize() != this->key.getKeySize())) {
		throw RUNTIME_ERROR("Key length is not correct for this hashAlgorithm");
	}

	ctx = HMAC_CTX_new();
	if (unlikely(!ctx)) {
		throw RUNTIME_ERROR("Failed HMAC_CTX_new()");
	}

	const int keyLenghthAsInt = Gicame::Utilities::safeNumericCast<int>(this->key.getKeySize());

	if (unlikely(!HMAC_Init_ex(
		ctx, this->key.get(), keyLenghthAsInt, hashAlgorithm.toEvpMd(), NULL
	))) {
		HMAC_CTX_free(ctx);
		ctx = NULL;
		throw RUNTIME_ERROR("Failed HMAC_Init_ex()");
	}
}

HmacCalculator::~HmacCalculator() {
	if (likely(ctx)) {
		HMAC_CTX_free(ctx);
	}
}

void HmacCalculator::update(const std::vector<byte_t>& source) {
	if (unlikely(!ctx)) {
		throw RUNTIME_ERROR("ctx is NULL");
	}
	if (unlikely(source.size() > (size_t)INT_MAX)) {
		throw RUNTIME_ERROR("Source size too big");
	}
	if (unlikely(!HMAC_Update(ctx, source.data(), source.size()))) {
		throw RUNTIME_ERROR("Failed HMAC_Update()");
	}
}

void HmacCalculator::update(const void* source, const size_t size) {
	if (unlikely(!ctx)) {
		throw RUNTIME_ERROR("ctx is NULL");
	}
	if (unlikely(size > (size_t)INT_MAX)) {
		throw RUNTIME_ERROR("Source size too big");
	}
	if (unlikely(!HMAC_Update(ctx, (byte_t*)source, size))) {
		throw RUNTIME_ERROR("Failed HMAC_Update()");
	}
}

std::vector<byte_t> HmacCalculator::finalize() {
	std::vector<byte_t> res(hashAlgorithm.hashSize());
	finalize(res.data());
	return res;
}

size_t HmacCalculator::finalize(void* dest) {
	if (unlikely(!ctx)) {
		throw RUNTIME_ERROR("ctx is NULL");
	}

	unsigned int hashSize;
	if (unlikely(!HMAC_Final(ctx, (byte_t*)dest, &hashSize))) {
		throw RUNTIME_ERROR("Failed HMAC_Final()");
	}
	if (unlikely(hashSize != hashAlgorithm.hashSize())) {
		throw RUNTIME_ERROR("Unexpected hashSize");
	}

	// Clean up
	HMAC_CTX_free(ctx);
	ctx = NULL;

	return hashSize;
}

std::vector<byte_t> HmacCalculator::hmac(
	const SymmetricKey& key, const HashAlgorithm hashAlgorithm, const void* source,
	const size_t size
) {
	HmacCalculator hmacCalc(key, hashAlgorithm);
	hmacCalc.update(source, size);
	return hmacCalc.finalize();
}

size_t HmacCalculator::hmac(
	const SymmetricKey& key, const HashAlgorithm hashAlgorithm, const void* source,
	const size_t size, void* out
) {
	HmacCalculator hmacCalc(key, hashAlgorithm);
	hmacCalc.update(source, size);
	return hmacCalc.finalize(out);
}
