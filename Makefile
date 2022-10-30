CC           = g++
FLAGS        = -std=c++17 -DGICAME_EXPORTS -I./headers -lstdc++ -L/usr/lib -L./build
FLAGS_CRYPTO = -std=c++17 -DGICAME_CRYPTO_EXPORTS -I./headers -lstdc++ -L/usr/lib -L./build -I/usr/lib/openssl-1.0/include -L/usr/lib/openssl-1.0/lib -lssl -lcrypto
CFLAGS       = -fPIC -Wall -Wextra
RELEASEFLAGS = -O2

HEADERS = $(echo headers/*.h)
GICAME_OBJ_FILES = build/TcpSocket.o build/TcpListeningSocket.o build/Serial.o build/RpcClient.o build/RpcServer.o build/NetworkDefinitions.o build/ErrorHandling.o
GICAME_CRYPTO_OBJ_FILES = build/X509Store.o build/X509Certificate.o build/Signer.o build/Verifier.o build/Decryptor.o build/Encryptor.o build/HashCalculator.o build/HashCalculator.o build/EvpKey.o build/SymmetricKey.o build/Random.o


### Build all ###

all: build/libgicame.so build/libgicamecrypto.so build/RpcClientOverTcp build/RpcServerOverTcp
	echo "Building Gicame and Gicame-crypto"


### Gicame OBJ files ###

build/TcpSocket.o: src/network/TcpSocket.cpp headers/network/TcpSocket.h $(HEADERS)
	$(CC) $(FLAGS) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

build/TcpListeningSocket.o: src/network/TcpListeningSocket.cpp headers/network/TcpListeningSocket.h $(HEADERS)
	$(CC) $(FLAGS) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

build/NetworkDefinitions.o: src/network/NetworkDefinitions.cpp headers/network/NetworkDefinitions.h $(HEADERS)
	$(CC) $(FLAGS) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

build/Serial.o: src/device/Serial.cpp headers/device/Serial.h $(HEADERS)
	$(CC) $(FLAGS) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

build/RpcClient.o: src/rpc/RpcClient.cpp headers/rpc/RpcClient.h $(HEADERS)
	$(CC) $(FLAGS) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

build/RpcServer.o: src/rpc/RpcServer.cpp headers/rpc/RpcServer.h $(HEADERS)
	$(CC) $(FLAGS) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

build/ErrorHandling.o: src/os/ErrorHandling.cpp headers/os/ErrorHandling.h $(HEADERS)
	$(CC) $(FLAGS) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@
	

### Gicame-crypto OBJ files ###

build/X509Store.o: src/crypto/certificate/X509Store.cpp headers/crypto/certificate/X509Store.h $(HEADERS)
	$(CC) $(FLAGS_CRYPTO) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

build/X509Certificate.o: src/crypto/certificate/X509Certificate.cpp headers/crypto/certificate/X509Certificate.h $(HEADERS)
	$(CC) $(FLAGS_CRYPTO) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

build/Signer.o: src/crypto/ds/Signer.cpp headers/crypto/ds/Signer.h $(HEADERS)
	$(CC) $(FLAGS_CRYPTO) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

build/Verifier.o: src/crypto/ds/Verifier.cpp headers/crypto/ds/Verifier.h $(HEADERS)
	$(CC) $(FLAGS_CRYPTO) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

build/Decryptor.o: src/crypto/encryption/Decryptor.cpp headers/crypto/encryption/Decryptor.h $(HEADERS)
	$(CC) $(FLAGS_CRYPTO) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

build/Encryptor.o: src/crypto/encryption/Encryptor.cpp headers/crypto/encryption/Encryptor.h $(HEADERS)
	$(CC) $(FLAGS_CRYPTO) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

build/HashCalculator.o: src/crypto/hash/HashCalculator.cpp headers/crypto/hash/HashCalculator.h $(HEADERS)
	$(CC) $(FLAGS_CRYPTO) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

build/HmacCalculator.o: src/crypto/hash/HmacCalculator.cpp headers/crypto/hash/HmacCalculator.h $(HEADERS)
	$(CC) $(FLAGS_CRYPTO) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

build/EvpKey.o: src/crypto/key/EvpKey.cpp headers/crypto/key/EvpKey.h $(HEADERS)
	$(CC) $(FLAGS_CRYPTO) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

build/SymmetricKey.o: src/crypto/key/SymmetricKey.cpp headers/crypto/key/SymmetricKey.h $(HEADERS)
	$(CC) $(FLAGS_CRYPTO) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

build/Random.o: src/crypto/random/Random.cpp headers/crypto/random/Random.h $(HEADERS)
	$(CC) $(FLAGS_CRYPTO) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@


### Gicame library ###

build/libgicame.so: $(GICAME_OBJ_FILES)
	# Building the shared library
	$(CC) $(FLAGS) $^ $(CFLAGS) $(RELEASEFLAGS) -shared -o $@

build/libgicame.a: $(GICAME_OBJ_FILES)
	# This command creates the static library.
	# "r" means to insert with replacement,
	# "c" means to create a new archive, and
	# "s" means to write an index.
	ar rcs $^


### Gicame-crypto library ###

build/libgicamecrypto.so: $(GICAME_CRYPTO_OBJ_FILES)
	# Building the shared library
	$(CC) $(FLAGS_CRYPTO) $^ $(CFLAGS) $(RELEASEFLAGS) -shared -o $@

build/libgicamecrypto.a: $(GICAME_CRYPTO_OBJ_FILES)
	# This command creates the static library.
	# "r" means to insert with replacement,
	# "c" means to create a new archive, and
	# "s" means to write an index.
	ar rcs $^


### Some examples ###

build/RpcClientOverTcp: examples/RpcClientOverTcp/main.cpp examples/RpcServerOverTcp/RemoteFunctions.h
	$(CC) $(FLAGS) $< build/libgicame.so $(CFLAGS) $(RELEASEFLAGS) -o $@

build/RpcServerOverTcp: examples/RpcServerOverTcp/main.cpp examples/RpcServerOverTcp/RemoteFunctions.h
	$(CC) $(FLAGS) $< build/libgicame.so $(CFLAGS) $(RELEASEFLAGS) -o $@


### Clean ###

clean:
	-rm build/*
	-touch build/.gitkeep
