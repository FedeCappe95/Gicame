
#ifndef __VERIFIER_H__
#define __VERIFIER_H__


#include <vector>
#include "common.h"
#include "../key/EvpKey.h"


struct evp_md_ctx_st;
struct evp_md_st;


namespace Gicame::Crypto {

	class GICAME_CRYPTO_API Verifier {

	private:
		evp_md_ctx_st* ctx;
		const evp_md_st* digestAlg;

	public:
		Verifier();
		~Verifier();
		void update(const std::vector<byte_t>& source);
		void update(const void* source, const size_t size);
		bool verify(EvpKey pubKey, const std::vector<byte_t>& sign);
		bool verify(EvpKey pubKey, const void* sign, const size_t size);

	public:
		static bool staticVerification(EvpKey pubKey, const std::vector<byte_t>& source, const std::vector<byte_t>& sign);
		static bool staticVerification(EvpKey pubKey, const void* source, const size_t sourceSize, const void* sign, const size_t signSize);

	};

};


#endif
