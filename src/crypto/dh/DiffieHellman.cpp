#include <openssl/dh.h>
#include "crypto/dh/DiffieHellman.h"
#include "crypto/hash/HashCalculator.h"


using namespace Gicame::Crypto;



DiffieHellman::DiffieHellman() : dh(NULL)
{
	// Nothing to do
}

DiffieHellman::~DiffieHellman() {
	if (likely(dh))
		DH_free(dh);
}

void DiffieHellman::createSession(const std::vector<byte_t>& generator, const std::vector<byte_t>& primeModulus) {
	if (unlikely(dh))
		DH_free(dh);

    dh = DH_new();
    if (unlikely(!dh))
        throw RUNTIME_ERROR("DH_new(...) failed");

    BIGNUM* pBn = BN_bin2bn(primeModulus.data(), Gicame::Utilities::safeNumericCast<int>(primeModulus.size()), NULL);
    BIGNUM* gBn = BN_bin2bn(generator.data(), Gicame::Utilities::safeNumericCast<int>(generator.size()), NULL);
    if (unlikely(!pBn || !gBn || !DH_set0_pqg(dh, pBn, NULL, gBn))) {
        DH_free(dh);
        BN_free(pBn);
        BN_free(gBn);
        throw RUNTIME_ERROR("Invalid DiffieHellman parameters");
    }

    int ec;
    if (unlikely(DH_check(dh, &ec) != 1))
        throw RUNTIME_ERROR("Invalid DiffieHellman parameters");

    DH_generate_key(dh);
}

size_t DiffieHellman::getSize() {
    if (unlikely(!dh))
        return 0;
    return (size_t)DH_size(dh);
}

std::vector<byte_t> DiffieHellman::getPublicComponent() {
    const BIGNUM* pubKeyBn;
    DH_get0_key(dh, &pubKeyBn, NULL);

    const int byteSize = BN_num_bytes(pubKeyBn);

    std::vector<byte_t> pubKey(byteSize);
    pubKey.resize(BN_bn2bin(pubKeyBn, pubKey.data()));

    return pubKey;
}

std::vector<byte_t> DiffieHellman::computeSharedSecret(const std::vector<byte_t>& otherPeerPublicComponent) {
    BIGNUM* otherPeerPubKeyBn = BN_bin2bn(
        otherPeerPublicComponent.data(),
        Gicame::Utilities::safeNumericCast<int>(otherPeerPublicComponent.size()),
        BN_new()
    );

    std::vector<byte_t> sharedSecret(DH_size(dh));
    const int sharedSecretSize = DH_compute_key(sharedSecret.data(), otherPeerPubKeyBn, dh);
    sharedSecret.resize(sharedSecretSize);

    return sharedSecret;
}

std::vector<byte_t> DiffieHellman::computeSharedSecret(const std::vector<byte_t>& otherPeerPublicComponent, const HashAlgorithm hashAlgorithm) {
    const std::vector<byte_t> sharedSecret = computeSharedSecret(otherPeerPublicComponent);
    return HashCalculator::hash(hashAlgorithm, sharedSecret.data(), sharedSecret.size());
}
