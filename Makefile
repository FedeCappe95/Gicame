CC           = gcc
FLAGS        = -std=c++17 -DGICAME_EXPORTS -I./headers -lstdc++ -L/usr/lib
CFLAGS       = -fPIC -Wall -Wextra -march=native
RELEASEFLAGS = -O2

HEADERS = $(echo headers/*.h)
OBJ_FILES = build/TcpSocket.o


build/TcpSocket.o: src/network/TcpSocket.cpp headers/network/TcpSocket.h $(HEADERS)
	$(CC) $< $(FLAGS) $(CFLAGS) $(RELEASEFLAGS) -c -o $@

build/libgicame.so: $(OBJ_FILES)
	# Building the shared library
	$(CC) $^ $(FLAGS) $(CFLAGS) $(RELEASEFLAGS) -shared -o $@

build/libgicame.a: $(OBJ_FILES)
	# This command creates the static library.
	# "r" means to insert with replacement,
	# "c" means to create a new archive, and
	# "s" means to write an index.
	ar rcs $^

clean:
	-rm build/*.so
	-rm build/*.o
	-rm build/*.a
	-rm build/*.out
