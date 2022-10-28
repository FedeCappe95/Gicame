#ifndef __HMACCALCULATOR_H__
#define __HMACCALCULATOR_H__


#include <vector>
#include <string.h>
#include "common.h"
#include "HashAlgorithm.h"
#include "../key/SymmetricKey.h"


struct hmac_ctx_st;


namespace Gicame::Crypto {

	class GICAME_CRYPTO_API HmacCalculator {

	private:
		SymmetricKey key;
		const HashAlgorithm hashAlgorithm;
		hmac_ctx_st* ctx;

	public:
		HmacCalculator(const SymmetricKey& key, const HashAlgorithm hashAlgorithm);
		~HmacCalculator();
		void updateHash(const std::vector<byte_t>& source);
		void updateHash(const void* source, const size_t size);
		std::vector<byte_t> getHash();
		size_t getHash(void* dest);

	public:
		static std::vector<byte_t> staticHash(const SymmetricKey& key, const HashAlgorithm hashAlgorithm, const void* source, const size_t size);
		static size_t staticHash(const SymmetricKey& key, const HashAlgorithm hashAlgorithm, const void* source, const size_t size, void* out);

	};

};


#endif
