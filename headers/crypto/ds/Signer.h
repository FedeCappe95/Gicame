#ifndef __SIGNER_H__
#define __SIGNER_H__


#include <vector>
#include <string.h>
#include "common.h"
#include "../key/EvpKey.h"


struct evp_md_ctx_st;
struct evp_md_st;


namespace Gicame::Crypto {

	class GICAME_CRYPTO_API Signer {

	private:
		const evp_md_st* digestAlg;
		evp_md_ctx_st* ctx;

	public:
		Signer();
		~Signer();
		void update(const std::vector<byte_t>& source);
		void update(const void* source, const size_t sourceSize);
		std::vector<byte_t> sign(EvpKey privKey);
		size_t sign(EvpKey privKey, void* outSign);

	public:
		static std::vector<byte_t> staticSign(EvpKey privKey, const std::vector<byte_t>& source);
		static std::vector<byte_t> staticSign(EvpKey privKey, const void* source, const size_t sourceSize);
		static size_t staticSign(EvpKey privKey, const void* source, const size_t sourceSize, void* dest);

	};

};


#endif
