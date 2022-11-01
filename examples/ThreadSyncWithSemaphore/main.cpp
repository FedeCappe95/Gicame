#include <iostream>
#include <stdint.h>
#include <vector>
#include <thread>
#include <chrono>
#include "../../headers/sm/Semaphore.h"


using namespace Gicame;


constexpr size_t SEM_MAX_CONCURRENCY = 1u;
constexpr char SEM_NAME[] = "HelloSem";



static void threadBody() {
	Semaphore sem(SEM_NAME, SEM_MAX_CONCURRENCY);
	sem.acquire();
	std::cout << "I'm the child thread and I'm doing dodgy stuff (DiodeGoneWild's critical section)" << std::endl;
	std::cout << "Bye from the child thread!" << std::endl;
	sem.release();
}


int main() {
	std::cout << "Gicame example: ThreadSyncWithSemaphore" << std::endl;

	// I can instantiate a Sempahore and acquire it or instantiate a semaphore with initialLockCount = SEM_MAX_CONCURRENCY
	Semaphore sem(SEM_NAME, SEM_MAX_CONCURRENCY);
	sem.acquire();

	std::thread t1(threadBody);  // t1 starts

	for (size_t i = 0; i < 5u; ++i) {
		std::cout << "Main thread is waiting..." << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
	}

	std::cout << "Main thread release the Semaphore" << std::endl;
	sem.release();

	// Join with t1
	t1.join();
	std::cout << "Bye from the main thread!" << std::endl;

	return 0;
}