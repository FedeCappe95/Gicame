CC           = g++
FLAGS        = -std=c++17 -DGICAME_EXPORTS -I./headers -lstdc++ -L/usr/lib -L./build
CFLAGS       = -fPIC -Wall -Wextra -march=native
RELEASEFLAGS = -O2

HEADERS = $(echo headers/*.h)
OBJ_FILES = build/TcpSocket.o build/TcpListeningSocket.o build/Serial.o build/RpcClient.o build/RpcServer.o build/NetworkDefinitions.o


all: build/libgicame.so build/RpcClientOverTcp build/RpcServerOverTcp
	echo "Building Gicame"

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

build/libgicame.so: $(OBJ_FILES)
	# Building the shared library
	$(CC) $(FLAGS) $^ $(CFLAGS) $(RELEASEFLAGS) -shared -o $@

build/libgicame.a: $(OBJ_FILES)
	# This command creates the static library.
	# "r" means to insert with replacement,
	# "c" means to create a new archive, and
	# "s" means to write an index.
	ar rcs $^

build/RpcClientOverTcp: examples/RpcClientOverTcp/main.cpp examples/RpcServerOverTcp/RemoteFunctions.h
	$(CC) $(FLAGS) -lgicame $< $(CFLAGS) $(RELEASEFLAGS) -o $@

build/RpcServerOverTcp: examples/RpcServerOverTcp/main.cpp examples/RpcServerOverTcp/RemoteFunctions.h
	$(CC) $(FLAGS) -lgicame $< $(CFLAGS) $(RELEASEFLAGS) -o $@

clean:
	-rm build/*.so
	-rm build/*.o
	-rm build/*.a
	-rm build/*.out
