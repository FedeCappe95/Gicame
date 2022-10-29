#include "crypto/certificate/X509Store.h"
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <string.h>
#include <stdio.h>
#include "common.h"


using namespace Gicame::Crypto;


X509Store::X509Store() {
    X509_STORE* nativeStore = X509_STORE_new();
    if (unlikely(!nativeStore)) {
        throw RUNTIME_ERROR("X509_STORE_new failed");
    }
    X509_STORE_set_flags(nativeStore, X509_V_FLAG_CRL_CHECK);
}

X509Store::~X509Store() {
	if(likely(nativeStore))
		X509_STORE_free(nativeStore);
}

X509Store& X509Store::addCertificate(X509Certificate& certificate) {
    // Prevent X509* deallocation by making and store a copy of sp in certSet
    certSet.insert(certificate.ptr);

    if (unlikely(!X509_STORE_add_cert(nativeStore, certificate.ptr.get()))) {
        throw RUNTIME_ERROR("X509_STORE_add_cert failed");
    }

    return *this;
}

X509Store& X509Store::addCrlFromPemFile(const std::string& filePath) {
    FILE* fp;
    fopen_s(&fp, filePath.c_str(), "r");
    if (unlikely(!fp)) {
        throw RUNTIME_ERROR("Invalid filePath");
    }

    X509_CRL* crl = PEM_read_X509_CRL(fp, NULL, NULL, NULL);
    fclose(fp);
    if (unlikely(!crl)) {
        throw RUNTIME_ERROR("PEM_read_X509_CRL failed");
    }

    if (unlikely(!X509_STORE_add_crl(nativeStore, crl))) {
        X509_CRL_free(crl);
        throw RUNTIME_ERROR("X509_STORE_add_crl failed");
    }

    return *this;
}

bool X509Store::verify(X509Certificate& certificate) {
    X509_STORE_CTX* ctx = X509_STORE_CTX_new();
    if (unlikely(!ctx)) {
        throw RUNTIME_ERROR("X509_STORE_CTX_new failed");
    }

    if (unlikely(!X509_STORE_CTX_init(ctx, nativeStore, certificate.ptr.get(), NULL))) {
        X509_STORE_CTX_free(ctx);
        throw RUNTIME_ERROR("X509_STORE_CTX_init failed");
    }

    int ret = X509_verify_cert(ctx);
    X509_STORE_CTX_free(ctx);
    return (ret == 1);
}
