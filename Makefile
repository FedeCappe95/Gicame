CC           = g++
FLAGS        = -std=c++17 -DGICAME_EXPORTS -I./headers -lstdc++ -L/usr/lib -L./build
FLAGS_CRYPTO = -std=c++17 -DGICAME_CRYPTO_EXPORTS -I./headers -lstdc++ -L/usr/lib -L./build -I/usr/lib/openssl-1.0/include -L/usr/lib/openssl-1.0/lib -lssl -lcrypto
CFLAGS       = -fPIC -Wall -Wextra -lrt
RELEASEFLAGS = -O2

OS           = $(subst /,-,$(shell uname -o))
BUILD_DIR    = build-$(OS)

SUB_DIRS         = $(sort $(dir $(wildcard ./src/*/)))
SUB_DIRS_CRYPTO  = $(sort $(dir $(wildcard ./src/crypto/*/)))

HEADERS = $(echo headers/*.h)
GICAME_OBJ_FILES = $(BUILD_DIR)/common.o $(BUILD_DIR)/device/Serial.o $(BUILD_DIR)/network/NetworkDefinitions.o $(BUILD_DIR)/network/NetworkUtility.o $(BUILD_DIR)/network/TcpListeningSocket.o $(BUILD_DIR)/network/TcpSocket.o $(BUILD_DIR)/os/ErrorHandling.o $(BUILD_DIR)/os/NamedPipe.o $(BUILD_DIR)/rpc/RpcClient.o $(BUILD_DIR)/rpc/RpcServer.o $(BUILD_DIR)/sm/SharedMemory.o $(BUILD_DIR)/sm/Semaphore.o $(BUILD_DIR)/concurrency/TaskExecutor.o $(BUILD_DIR)/concurrency/SLSPSCQueue.o $(BUILD_DIR)/concurrency/SPSCQueue.o $(BUILD_DIR)/concurrency/InterprocessSignal.o
GICAME_CRYPTO_OBJ_FILES = $(BUILD_DIR)/crypto/certificate/X509Certificate.o $(BUILD_DIR)/crypto/certificate/X509Store.o $(BUILD_DIR)/crypto/dh/DiffieHellman.o $(BUILD_DIR)/crypto/ds/Signer.o $(BUILD_DIR)/crypto/ds/Verifier.o $(BUILD_DIR)/crypto/encryption/Decryptor.o $(BUILD_DIR)/crypto/encryption/Encryptor.o $(BUILD_DIR)/crypto/hash/HashCalculator.o $(BUILD_DIR)/crypto/hash/HmacCalculator.o $(BUILD_DIR)/crypto/key/EvpKey.o $(BUILD_DIR)/crypto/key/SymmetricKey.o $(BUILD_DIR)/crypto/random/Random.o


### Build all ###

all: before bin/libgicame.so bin/libgicamecrypto.so
	@echo "Done"

before:
	@-mkdir $(BUILD_DIR)/
	@-mkdir $(BUILD_DIR)/concurrency/
	@-mkdir $(BUILD_DIR)/device/
	@-mkdir $(BUILD_DIR)/network/
	@-mkdir $(BUILD_DIR)/os/
	@-mkdir $(BUILD_DIR)/rpc/
	@-mkdir $(BUILD_DIR)/sm/
	@-mkdir $(BUILD_DIR)/crypto/
	@-mkdir $(BUILD_DIR)/crypto/certificate/
	@-mkdir $(BUILD_DIR)/crypto/dh/
	@-mkdir $(BUILD_DIR)/crypto/ds/
	@-mkdir $(BUILD_DIR)/crypto/encryption/
	@-mkdir $(BUILD_DIR)/crypto/hash/
	@-mkdir $(BUILD_DIR)/crypto/key/
	@-mkdir $(BUILD_DIR)/crypto/random/
	@-mkdir bin/


### Gicame OBJ files ###

$(BUILD_DIR)/%.o: src/%.cpp $(HEADERS)
	@echo Compiling $@
	@$(CC) $(FLAGS) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

$(BUILD_DIR)/device/%.o: src/device/%.cpp $(HEADERS)
	@echo Compiling $@
	@$(CC) $(FLAGS) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

$(BUILD_DIR)/network/%.o: src/network/%.cpp $(HEADERS)
	@echo Compiling $@
	@$(CC) $(FLAGS) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

$(BUILD_DIR)/os/%.o: src/os/%.cpp $(HEADERS)
	@echo Compiling $@
	@$(CC) $(FLAGS) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

$(BUILD_DIR)/rpc/%.o: src/rpc/%.cpp $(HEADERS)
	@echo Compiling $@
	@$(CC) $(FLAGS) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

$(BUILD_DIR)/sm/%.o: src/sm/%.cpp $(HEADERS)
	@echo Compiling $@
	@$(CC) $(FLAGS) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@


### Gicame-crypto OBJ files ###

$(BUILD_DIR)/crypto/%.o: src/crypto/%.cpp $(HEADERS)
	@echo Compiling $@
	@$(CC) $(FLAGS_CRYPTO) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

$(BUILD_DIR)/crypto/certificate/%.o: src/crypto/certificate/%.cpp $(HEADERS)
	@echo Compiling $@
	@$(CC) $(FLAGS_CRYPTO) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

$(BUILD_DIR)/crypto/dh/%.o: src/crypto/dh/%.cpp $(HEADERS)
	@echo Compiling $@
	@$(CC) $(FLAGS_CRYPTO) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

$(BUILD_DIR)/crypto/ds/%.o: src/crypto/ds/%.cpp $(HEADERS)
	@echo Compiling $@
	@$(CC) $(FLAGS_CRYPTO) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

$(BUILD_DIR)/crypto/encryption/%.o: src/crypto/encryption/%.cpp $(HEADERS)
	@echo Compiling $@
	@$(CC) $(FLAGS_CRYPTO) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

$(BUILD_DIR)/crypto/hash/%.o: src/crypto/hash/%.cpp $(HEADERS)
	@echo Compiling $@
	@$(CC) $(FLAGS_CRYPTO) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

$(BUILD_DIR)/crypto/key/%.o: src/crypto/key/%.cpp $(HEADERS)
	@echo Compiling $@
	@$(CC) $(FLAGS_CRYPTO) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@

$(BUILD_DIR)/crypto/random/%.o: src/crypto/random/%.cpp $(HEADERS)
	@echo Compiling $@
	@$(CC) $(FLAGS_CRYPTO) $< $(CFLAGS) $(RELEASEFLAGS) -c -o $@


### Gicame library ###

bin/libgicame.so: $(GICAME_OBJ_FILES)
	@echo Building the shared library
	@$(CC) $(FLAGS) $^ $(CFLAGS) $(RELEASEFLAGS) -shared -o $@

bin/libgicame.a: $(GICAME_OBJ_FILES)
	@ar rcs $^


### Gicame-crypto library ###

bin/libgicamecrypto.so: $(GICAME_CRYPTO_OBJ_FILES)
	@echo Building the shared library
	@$(CC) $(FLAGS_CRYPTO) $^ $(CFLAGS) $(RELEASEFLAGS) -shared -o $@

bin/libgicamecrypto.a: $(GICAME_CRYPTO_OBJ_FILES)
	@ar rcs $^


### Some examples ###
##
## build/Example-RpcClientOverTcp: examples/RpcClientOverTcp/main.cpp examples/RpcServerOverTcp/RemoteFunctions.h
## 	$(CC) $(FLAGS) $< build/libgicame.so $(CFLAGS) $(RELEASEFLAGS) -o $@
## 
## build/Example-RpcServerOverTcp: examples/RpcServerOverTcp/main.cpp examples/RpcServerOverTcp/RemoteFunctions.h
## 	$(CC) $(FLAGS) $< build/libgicame.so $(CFLAGS) $(RELEASEFLAGS) -o $@
## 
## build/Example-SharedMemory: examples/SharedMemory/main.cpp
## 	$(CC) $(FLAGS) $< build/libgicame.so $(CFLAGS) $(RELEASEFLAGS) -o $@


### Clean ###

clean:
	@-rm -f -r $(BUILD_DIR)/*
	@-rm -f -r bin/*
	@-mkdir $(BUILD_DIR)/
	@-mkdir $(BUILD_DIR)/concurrency/
	@-mkdir $(BUILD_DIR)/device/
	@-mkdir $(BUILD_DIR)/network/
	@-mkdir $(BUILD_DIR)/os/
	@-mkdir $(BUILD_DIR)/rpc/
	@-mkdir $(BUILD_DIR)/sm/
	@-mkdir $(BUILD_DIR)/crypto/
	@-mkdir $(BUILD_DIR)/crypto/certificate/
	@-mkdir $(BUILD_DIR)/crypto/dh/
	@-mkdir $(BUILD_DIR)/crypto/ds/
	@-mkdir $(BUILD_DIR)/crypto/encryption/
	@-mkdir $(BUILD_DIR)/crypto/hash/
	@-mkdir $(BUILD_DIR)/crypto/key/
	@-mkdir $(BUILD_DIR)/crypto/random/
	@-mkdir bin/
