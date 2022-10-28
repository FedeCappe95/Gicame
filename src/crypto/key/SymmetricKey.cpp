#include "crypto/key/SymmetricKey.h"
#include <string.h>


using namespace Gicame::Crypto;


SymmetricKey::SymmetricKey(const std::shared_ptr<const byte_t>& buffPtr, const size_t keySize) :
    buffPtr(buffPtr),
    keySize(keySize)
{
    // Nothing to do
}

const byte_t* SymmetricKey::get() const {
    return buffPtr.get();
}

size_t SymmetricKey::getKeySize() const {
    return keySize;
}

SymmetricKey SymmetricKey::createSymmetricKey(
    const byte_t* keyBuffer, const size_t keyBufferSize, const IAlgorithmDescriptor& algorithm
) {
    const size_t keySize = algorithm.keySize();
    if (unlikely(keyBufferSize < keySize)) {
        throw RUNTIME_ERROR("keyBuffer too short");
    }
    byte_t* keyBufferCopy = new byte_t[keySize];
    memcpy(keyBufferCopy, keyBuffer, keySize);
    auto deleter = [=](const byte_t* ptr) {
        Utilities::eraseMemory((byte_t*)ptr, keySize);
        delete[] ptr;
    };
    return SymmetricKey(std::shared_ptr<const byte_t>(keyBufferCopy, deleter), keySize);
}

SymmetricKey SymmetricKey::createSymmetricKeyTakeOwnership(
    const byte_t* keyBuffer, const size_t keyBufferSize, const IAlgorithmDescriptor& algorithm
) {
    const size_t keySize = algorithm.keySize();
    if (unlikely(keyBufferSize < keySize)) {
        throw RUNTIME_ERROR("keyBuffer too short");
    }
    auto deleter = [=](const byte_t* ptr) {
        Utilities::eraseMemory((byte_t*)ptr, keySize);
        delete[] ptr;
    };
    return SymmetricKey(std::shared_ptr<const byte_t>(keyBuffer, deleter), keySize);
}
