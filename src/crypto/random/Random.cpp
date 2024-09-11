#include "crypto/random/Random.h"
#include "utils/Numbers.h"
#include <climits>
#include <openssl/rand.h>


using namespace Gicame::Crypto::Random;


byte_t* getRandomBytes(void* buffer, const size_t size) {
	const int intSize = Gicame::Utilities::safeNumericCast<int>(size);
	int success;
	do {
		success = RAND_bytes((byte_t*)buffer, intSize);
	} while(success != 1);
	return (byte_t*)buffer;
}
