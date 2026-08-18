#ifndef __GICAME__CONCURRENCY__SIGNAL_H__
#define __GICAME__CONCURRENCY__SIGNAL_H__


#include <stdint.h>
#include <atomic>
#include <mutex>
#include <condition_variable>


namespace Gicame::Concurrency {

	/**
	 * @brief Inter-thread anonymous signal
	 *
	 * Signaling is sticky: once signaled another signal has no effects and there are no counters
	 * (it only has two levels 0 and 1).
	 * Optimized to minimize the occurrences of context switch.
	 * A timeout value of 0 ms means the wait acts as "tryWait", no other sentilell values exist
	 * (so no INFINITE constants such as in case of Windows Events).
	 */
	class Signal {

	private:   // Private data members
		std::atomic<uint32_t> e;
		std::mutex mtx;
		std::condition_variable eventFired;

	private:   // Private methods
		Signal(const Signal& o) = delete;
		Signal& operator=(const Signal& o) = delete;
		bool tryConsume() noexcept;

	public:    // Public methods
		Signal();
		~Signal() = default;
		void wait();
		bool wait(uint32_t timeoutMs);
		void signal();

	};


	/*
	 * ===== Inline implementation =====
	 */

	inline Signal::Signal() : e(0u) {};

	inline bool Signal::tryConsume() noexcept {
		if (e.load(std::memory_order_relaxed) == 0u)
			return false;
		uint32_t expected = 1u;
		return e.compare_exchange_strong(expected, 0u);
	}

	inline void Signal::wait() {
		// Fast path
		if (tryConsume())
			return;

		std::unique_lock<std::mutex> lock(mtx);
		for (;;) {
			if (tryConsume())
				break;
			eventFired.wait(lock);
		}
	}

	inline bool Signal::wait(uint32_t timeoutMs) {
		// Fast path
		if (tryConsume())
			return true;

		if (timeoutMs == 0u)
			return false;

		auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);

		std::unique_lock<std::mutex> lock(mtx);
		for (;;) {
			if (tryConsume())
				return true;

			if (eventFired.wait_until(lock, deadline) == std::cv_status::timeout) {
				// One last check before returning timeout
				return tryConsume();
			}
		}
	}

	inline void Signal::signal() {
		e.store(1u);
		{
			std::unique_lock<std::mutex> lock(mtx);
			eventFired.notify_all();
		}
	}

};


#endif
