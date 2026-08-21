#include <iostream>
#include <stdint.h>
#include <thread>
#include "../../headers/concurrency/SPSCQueue.h"
#include "../../headers/concurrency/InterprocessQueue.h"
#include "../../headers/dataTransfer/TextSender.h"
#include "../../headers/dataTransfer/TextReceiver.h"


using namespace Gicame;
using namespace Gicame::Concurrency;


constexpr size_t QUEUE_SIZE = 8 * 1024 * 1024;   // 8MiB
constexpr size_t ELEM_COUNT = 64 * 1024 * 1024;  // 64Mi of size_t
SPSCQueue queue(QUEUE_SIZE);
InterprocessQueue ipcQueue("IPC1", QUEUE_SIZE, Gicame::Concurrency::ConcurrencyRole::MASTER);


void producerBody() {
	for (size_t i = 0; i < ELEM_COUNT; ++i)
		queue.push(&i, sizeof(i));
	std::cout << "producerBody -> done" << std::endl;
}


void consumerBody() {
	size_t errorCount = 0;
	size_t elem;
	for (size_t i = 0; i < ELEM_COUNT; ++i) {
		queue.pop(&elem, sizeof(elem));
		if (elem != i)
			++errorCount;
	}
	std::cout << "consumerBody -> errorCount: " << errorCount << std::endl;
}

void producerBodyIpc() {
	for (size_t i = 0; i < ELEM_COUNT; ++i)
		ipcQueue.push(&i, sizeof(i));
	std::cout << "producerBodyIpc -> done" << std::endl;
}

void consumerBodyIpc() {
	size_t errorCount = 0;
	size_t elem;
	for (size_t i = 0; i < ELEM_COUNT; ++i) {
		ipcQueue.pop(&elem, sizeof(elem));
		if (elem != i)
			++errorCount;
	}
	std::cout << "consumerBodyIpc -> errorCount: " << errorCount << std::endl;
}

void textProducerBody() {
	TextSender textSender(&queue);

	for (size_t i = 0; i < ELEM_COUNT; ++i)
		textSender.sendText(std::to_string(i));
	std::cout << "textProducerBody -> done" << std::endl;
}


void textConsumerBody() {
	TextReceiver textReceiver(&queue);

	size_t errorCount = 0;
	for (size_t i = 0; i < ELEM_COUNT; ++i) {
		const std::string elem = textReceiver.receiveText();
		if (elem != std::to_string(i))
			++errorCount;
	}
	std::cout << "textConsumerBody -> errorCount: " << errorCount << std::endl;
}


int main(int /*argc*/, char** /*argv*/) {
	std::cout << "Gicame example: SLSPSCQueue" << std::endl;

	std::thread producer(producerBody);
	std::thread consumer(consumerBody);
	producer.join();
	consumer.join();

	std::thread ipcProducer(producerBodyIpc);
	std::thread ipcConsumer(consumerBodyIpc);
	ipcProducer.join();
	ipcConsumer.join();

	std::thread textProducer(textProducerBody);
	std::thread textConsumer(textConsumerBody);
	textProducer.join();
	textConsumer.join();

	return 0;
}
