#!/bin/sh
rm -f tests.out
g++ -std=c++17 -DGICAME_EXPORTS -I./headers -lstdc++ -L/usr/lib -L./build main.cpp ./catch2/catch_amalgamated.cpp ../bin/libgicame.so -fPIC -Wall -Wextra -lrt -o tests.out
chmod +x tests.out
./tests.out
