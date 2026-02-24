#include <iostream>
#include <stdint.h>
#include <string.h>
#include <thread>
#include <chrono>
#include "../../headers/os/NamedPipe.h"


using namespace Gicame;
using namespace Gicame::Os;



int main(int /*argc*/, char** /*argv*/) {
	std::cout << "Gicame example: NamedPipeServer" << std::endl;

	NamedPipe pipe("ExamplePipeName");
	pipe.create(4 * 1024);  // 4KiB
	pipe.open();

	uint32_t a = 0, b = 0;
	pipe.receive(&a, sizeof(a));
	pipe.receive(&b, sizeof(b));
	std::cout << "Client sent a = " << a << ", b = " << b << std::endl;

	uint32_t c = a + b;
	pipe.send(&c, sizeof(c));
	pipe.close();

	std::cout << "Reply c = " << c << std::endl;

	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	pipe.unlink();

	return 0;
}
