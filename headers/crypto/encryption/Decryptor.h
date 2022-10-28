#ifndef __DECRYPTOR_H__
#define __DECRYPTOR_H__


#include <vector>
#include "common.h"
#include "EncryptionAlgorithm.h"
#include "../key/SymmetricKey.h"


namespace Gicame::Crypto {

	class GICAME_CRYPTO_API Decryptor {

	private:
		SymmetricKey key;
		void* ctx;
		size_t plaintextLen;

	public:
		Decryptor(const SymmetricKey& key, const EncryptionAlgorithm& algorithm, const byte_t* iv);
		~Decryptor();
		size_t getPlaintextLen();
		size_t decrypt(const void* source, const size_t sourceSize, void* out);
		std::vector<byte_t> decrypt(const void* source, const size_t sourceSize);
		size_t finalize(void* out);
		std::vector<byte_t> finalize();

	public:
		static size_t staticDecrypt(SymmetricKey key, const EncryptionAlgorithm& algorithm, const byte_t* iv, const void* source, const size_t sourceSize, void* dest);
		static std::vector<byte_t> staticDecrypt(SymmetricKey key, const EncryptionAlgorithm& algorithm, const byte_t* iv, const void* source, const size_t sourceSize);

	};

};


#endif
