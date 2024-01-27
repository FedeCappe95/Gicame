#include <iostream>
#include <stdint.h>
#include <string.h>
#include "../../headers/os/NamedPipe.h"


using namespace Gicame;
using namespace Gicame::Os;



int main(int argc, char* argv[]) {
	std::cout << "Gicame example: NamedPipeClient" << std::endl;

	NamedPipe pipe("ExamplePipeName");
	pipe.open();

	uint32_t a = 5, b = 10;
	pipe.send(&a, sizeof(a));
	pipe.send(&b, sizeof(b));
	std::cout << "I sent a = " << a << ", b = " << b << std::endl;

	uint32_t c = 0;
	pipe.receive(&c, sizeof(c));

	std::cout << "Reply c = " << c << std::endl;

	return 0;
}
