#ifndef __X509CERTIFICATE_H__
#define __X509CERTIFICATE_H__


#include <memory>
#include <vector>
#include "common.h"
#include "../key/EvpKey.h"


struct x509_st;


namespace Gicame::Crypto {

	class X509Certificate {

		MOVABLE_BUT_NOT_COPYABLE;

	private:
		std::shared_ptr<x509_st> ptr;
		X509Certificate();
		X509Certificate(x509_st* cert);

	public:
		x509_st* get();
		std::shared_ptr<x509_st> getSp();
		GICAME_CRYPTO_API std::vector<byte_t> toDer() const;
		GICAME_CRYPTO_API std::vector<byte_t> toPem() const;
		GICAME_CRYPTO_API EvpKey getPublicKey() const;
		GICAME_CRYPTO_API void sign(EvpKey privKey);
		GICAME_CRYPTO_API std::string getSubjectName() const;
		GICAME_CRYPTO_API X509Certificate& setSubjectName(const std::string& field, const std::string& value);
		GICAME_CRYPTO_API X509Certificate& addSubjectName(const std::string& field, const std::string& value);
		GICAME_CRYPTO_API std::string getIssuerName() const;
		GICAME_CRYPTO_API X509Certificate& setIssuerName(const std::string& field, const std::string& value);
		GICAME_CRYPTO_API X509Certificate& addIssuerName(const std::string& field, const std::string& value);

	public:
		GICAME_CRYPTO_API static X509Certificate fromLegacyX509(x509_st* cert);
		GICAME_CRYPTO_API static X509Certificate newEmptyX509Certificate();
		GICAME_CRYPTO_API static X509Certificate fromDer(std::vector<byte_t> der);
		GICAME_CRYPTO_API static X509Certificate fromPem(std::vector<byte_t> pem);

	};

};


#endif
