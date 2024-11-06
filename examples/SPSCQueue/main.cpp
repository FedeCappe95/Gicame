#include <iostream>
#include <stdint.h>
#include <thread>
#include "../../headers/concurrency/SLSPSCQueue.h"


using namespace Gicame;
using namespace Gicame::Concurrency;


uint8_t buffer[8 * 1024 * 1024];  // 8MiB buffer
SLSPSCQueue queue(buffer, sizeof(buffer), Gicame::Concurrency::ConcurrencyRole::MASTER);
constexpr size_t ELEM_COUNT = 64 * 1024 * 1024;  // 64Mi of size_t


void producerBody() {
	for (size_t i = 0; i < ELEM_COUNT; ++i)
		queue.push(i);
	std::cout << "producerBody -> done" << std::endl;
}


void consumerBody() {
	size_t errorCount = 0;
	for (size_t i = 0; i < ELEM_COUNT; ++i) {
		const size_t elem = queue.pop<size_t>();
		if (elem != i)
			++errorCount;
	}
	std::cout << "consumerBody -> errorCount: " << errorCount << std::endl;
}


int main(int argc, char* argv[]) {
	std::cout << "Gicame example: SLSPSCQueue" << std::endl;

	std::thread producer(producerBody);
	std::thread consumer(consumerBody);

	producer.join();
	consumer.join();

	return 0;
}
