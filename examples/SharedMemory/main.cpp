#include <iostream>
#include <stdint.h>
#include <string.h>
#include <thread>
#include <chrono>
#include "../../headers/sm/SharedMemory.h"


using namespace Gicame;


struct MyData {
	uint32_t a;
	float b;
	char message[256];
	std::atomic<bool> readDone;
};


static void printMyData(const MyData& myData) {
	std::cout << "MyData:\n" << "  a: " << myData.a << "\n  b: " << myData.b;
	std::cout << "\n  message: " << myData.message << std::endl;
}

static void myStrCpy(char* dst, const size_t len, const char* src) {
#ifdef MSVC
	strcpy_s(dst, len, src);
#else
	UNUSED(len);
	strcpy(dst, src);
#endif
}


static void peer0() {
	SharedMemory sm("MyMemory", sizeof(MyData));
	sm.open(true);
	MyData* myData = sm.getAs<MyData>(true);
	myData->a = 5;
	myData->b = 12.0;
	myData->readDone = false;
	myStrCpy(myData->message, sizeof(MyData::message), "Hello SharedMemory!");
	std::cout << "Waiting the other peer" << std::endl;
	while (!myData->readDone) {
		std::cout << ".";
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	std::cout << "\nDone" << std::endl;
	sm.close();
}

static void peer1() {
	SharedMemory sm("MyMemory", sizeof(MyData));
	sm.open(false);
	MyData* myData = sm.getAs<MyData>(false);
	printMyData(*myData);
	myData->readDone = true;
	sm.close();
}


int main(int argc, char* argv[]) {
	std::cout << "Gicame example: SharedMemory" << std::endl;

	if (argc < 2) {
		std::cout << "Usage:\n  Example-SharedMemory -peer0\n  Example-SharedMemory -peer1" << std::endl;
		return 1;
	}

	const std::string role = argv[1];

	if (role == "-peer0") {
		peer0();
	}
	else if (role == "-peer1") {
		peer1();
	}
	else {
		std::cout << "Usage:\n  Example-SharedMemory -peer0\n  ExampleSharedMemory -peer1" << std::endl;
		return 1;
	}

	return 0;
}
