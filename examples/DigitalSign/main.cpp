#include <iostream>
#include <stdint.h>
#include <vector>
#include <fstream>
#include "../../headers/io/Io.h"
#include "../../headers/crypto/ds/Signer.h"
#include "../../headers/crypto/ds/Verifier.h"
#include "../../headers/crypto/certificate/X509Certificate.h"


using namespace Gicame;
using namespace Gicame::Crypto;


static void printDigitalSign(const std::vector<byte_t>& ds) {
	std::cout << "DigitalSign: ";
	for (const byte_t b : ds)
		std::cout << (uint32_t)b << " ";
	std::cout << std::endl;
}


int main() {
	std::cout << "Gicame example: DigitalSign" << std::endl;

	std::string filePath;
	std::cout << "File path: ";
	std::cin >> filePath;

	const std::vector<byte_t> fileContent = Gicame::IO::readFileContent(filePath);

	// Make the digital sign
	const EvpKey privKey = EvpKey::fromPrivateKeyPemFile("../../../examples-assets/KeyPairExample/privKey.pem");
	const std::vector<byte_t> digitalSign = Signer::sign(privKey, fileContent.data(), fileContent.size());
	printDigitalSign(digitalSign);

	// Verify the digital sign
	const X509Certificate certificate = X509Certificate::fromPemFile("../../../examples-assets/KeyPairExample/cert.pem");
	const EvpKey pubKey = certificate.getPublicKey();
	const bool verified = Verifier::verify(pubKey, fileContent, digitalSign);

	std::cout << "Digital sign is " << (verified ? "" : "not ") << "verified" << std::endl;

	return 0;
}