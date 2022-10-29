#include <iostream>
#include <stdint.h>
#include <vector>
#include <fstream>
#include "../../headers/io/Io.h"
#include "../../headers/crypto/encryption/Encryptor.h"
#include "../../headers/crypto/encryption/Decryptor.h"


using namespace Gicame;
using namespace Gicame::Crypto;


#define MAX_FILE_SIZE (size_t)(128*1024*1024)  // 128MB


static byte_t iv[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
static byte_t keyBuffer[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};


int main() {
	std::cout << "Gicame example: EncryptDecryptFile" << std::endl;

	std::string filePath;
	std::cout << "File path: ";
	std::cin >> filePath;

	const EncryptionAlgorithm ea = EncryptionAlgorithm::AES_128_CBC;
	const SymmetricKey key = SymmetricKey::createSymmetricKey(keyBuffer, sizeof(keyBuffer), ea);

	const std::vector<byte_t> fileContent = Gicame::IO::readFileContent(filePath);
	const std::vector<byte_t> encrypted = Encryptor::encrypt(key, ea, iv, fileContent.data(), fileContent.size());
	const std::vector<byte_t> decrypted = Decryptor::decrypt(key, ea, iv, encrypted.data(), encrypted.size());

	std::cout << "encrypted and decrypted are " << (encrypted == decrypted ? "" : "not ") << "equal"  << std::endl;
	std::cout << "fileContent and decrypted are " << (fileContent == decrypted ? "" : "not ") << "equal"  << std::endl;

	Gicame::IO::writeFileContent(filePath + ".enc", encrypted);

	return 0;
}