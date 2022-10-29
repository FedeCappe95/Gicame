#include <iostream>
#include <stdint.h>
#include <vector>
#include <fstream>
#include "../../headers/crypto/ds/Signer.h"
#include "../../headers/crypto/ds/Verifier.h"
#include "../../headers/crypto/certificate/X509Certificate.h"


using namespace Gicame;
using namespace Gicame::Crypto;


#define MAX_FILE_SIZE (size_t)(128*1024*1024)  // 128MB


static byte_t iv[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
static byte_t keyBuffer[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};


static std::ifstream::pos_type fileSize(const std::string& filePath) {
	std::ifstream in(filePath, std::ifstream::ate | std::ifstream::binary);
	return in.tellg();
}

static std::vector<byte_t> readFileContent(const std::string& filePath) {
	std::ifstream inFile;
	const size_t size = std::min((size_t)fileSize(filePath), MAX_FILE_SIZE);
	inFile.open(filePath, std::ios::binary | std::ios::in);
	if (!inFile.is_open()) {
		std::cerr << "Error opening file" << std::endl;
		exit(1);
	}
	std::vector<byte_t> content(size);
	inFile.read((char*)content.data(), size);
	std::ifstream::pos_type realFileSize = inFile.tellg();
	inFile.close();
	return content;
}


static void printDigitalSign(const std::vector<byte_t>& ds) {
	std::cout << "DigitalSign: ";
	for (const byte_t b : ds)
		std::cout << (uint32_t)b;
	std::cout << std::endl;
}


int main() {
	std::cout << "Gicame example: DigitalSign" << std::endl;

	std::string filePath;
	std::cout << "File path: ";
	std::cin >> filePath;

	const std::vector<byte_t> fileContent = readFileContent(filePath);

	// Make the digital sign
	const EvpKey privKey = EvpKey::fromPrivateKeyPem(readFileContent("../../../examples/KeyPairExample/privKey.pem"));
	const std::vector<byte_t> digitalSign = Signer::sign(privKey, fileContent.data(), fileContent.size());
	printDigitalSign(digitalSign);

	// Verify the digital sign
	const X509Certificate certificate = X509Certificate::fromPem(readFileContent("../../../examples/KeyPairExample/cert.pem"));
	const EvpKey pubKey = certificate.getPublicKey();
	const bool verified = Verifier::verify(pubKey, fileContent, digitalSign);

	std::cout << "Digital sign is " << (verified ? "" : "not ") << "verified" << std::endl;

	return 0;
}