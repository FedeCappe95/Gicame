#ifndef __GICAME__CONCURRENCY__SIGNAL_H__
#define __GICAME__CONCURRENCY__SIGNAL_H__


#include "../common.h"
#include "./WaitResult.h"
#include "../utils/NotCopyable.h"
#include <stdint.h>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <chrono>


namespace Gicame::Concurrency {

	/**
	 * @brief Inter-thread anonymous signal
	 *
	 * Signaling is sticky: once signaled another signal has no effects and there are no counters
	 * (it only has two levels 0 and 1).
	 * Optimized to minimize the occurrences of context switch.
	 * A timeout value of 0 ms means the wait acts as "tryWait", no other sentilel values exist
	 * (so no INFINITE constants such as in case of Windows Events).
	 * Do not destroy Signal while other threads are waiting or signaling.
	 */
	class Signal {

		NOT_COPYABLE(Signal);

	private:   // Private data members
		std::atomic<uint32_t> e;
		std::mutex mtx;
		std::condition_variable eventFired;

	private:   // Private methods
		bool tryConsume() noexcept;

	public:    // Public methods
		Signal();
		~Signal() = default;
		void wait();
		bool wait(uint32_t timeoutMs);
		void signal();

	};


	template <size_t SIG_COUNT>
	class SignalGroup;


	/**
	 * @brief Proxy to a single signal of a SignalGroup
	 *
	 * It exposes the very same API as Signal, so that the two types can be used interchangeably
	 * (e.g. by generic code templated on the signal type).
	 * Instances are obtained calling SignalGroup::getSignal().
	 */
	template <size_t SIG_COUNT>
	class SignalOfAGroup {

		friend class SignalGroup<SIG_COUNT>;

	private:   // Private data members
		SignalGroup<SIG_COUNT>* group;
		size_t signalIndex;

	private:   // Private methods
		constexpr SignalOfAGroup(SignalGroup<SIG_COUNT>& group_, const size_t signalIndex_) noexcept;

	public:    // Public methods
		SignalOfAGroup(const SignalOfAGroup& o) = default;
		SignalOfAGroup& operator=(const SignalOfAGroup& o) = default;
		~SignalOfAGroup() = default;
		void wait();
		bool wait(uint32_t timeoutMs);
		void signal();

	};


	/**
	 * @brief Group of SIG_COUNT inter-thread anonymous signals sharing one mutex and one
	 *        condition variable
	 *
	 * Each signal of the group behaves exactly as a standalone Signal (sticky, two levels only,
	 * a timeout value of 0 ms means the wait acts as "tryWait").
	 * Sharing the synchronization primitives allows waitOne() to wait for any of the signals at
	 * once: it consumes exactly one signal and returns its index, or NO_SIGNALS on timeout.
	 * When more than one signal is pending, waitOne() returns the lowest index: a signal can
	 * be starved by a lower index one that is signaled at a higher rate.
	 * getSignal() returns a Signal-like proxy to one of the signals of the group.
	 * Do not destroy SignalGroup while other threads are waiting or signaling.
	 * Destroying SignalGroup make unusuable all related SignalOfAGroup, using them is UB.
	 */
	template <size_t SIG_COUNT>
	class SignalGroup {

		NOT_COPYABLE(SignalGroup);

	public:   // Public constants
		static constexpr size_t NO_SIGNALS = ~size_t(0);

	private:   // Private data members
		std::atomic<uint32_t> events[SIG_COUNT];
		std::mutex mtx;
		std::condition_variable eventFired;

	private:   // Private methods
		bool tryConsume(size_t signalIndex) noexcept;
		size_t tryConsumeOne() noexcept;

	public:    // Public methods
		SignalGroup();
		~SignalGroup() = default;
		void wait(size_t signalIndex);
		bool wait(size_t signalIndex, uint32_t timeoutMs);
		size_t waitOne();
		size_t waitOne(uint32_t timeoutMs);
		void signal(size_t signalIndex);
		SignalOfAGroup<SIG_COUNT> getSignal(size_t signalIndex);
		template <size_t SIGNAL_INDEX> constexpr SignalOfAGroup<SIG_COUNT> getSignal() noexcept;

	};


	/*
	 * ===== Signal inline implementation =====
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


	/*
	 * ===== SignalGroup inline implementation =====
	 */

	template <size_t SIG_COUNT>
	inline SignalGroup<SIG_COUNT>::SignalGroup() {
		for (size_t i = 0; i < SIG_COUNT; ++i)
			events[i].store(0u, std::memory_order_relaxed);
	}

	template <size_t SIG_COUNT>
	inline bool SignalGroup<SIG_COUNT>::tryConsume(size_t signalIndex) noexcept {
		if (events[signalIndex].load(std::memory_order_relaxed) == 0u)
			return false;
		uint32_t expected = 1u;
		return events[signalIndex].compare_exchange_strong(expected, 0u);
	}

	template <size_t SIG_COUNT>
	inline size_t SignalGroup<SIG_COUNT>::tryConsumeOne() noexcept {
		for (size_t i = 0; i < SIG_COUNT; ++i) {
			if (tryConsume(i))
				return i;
		}
		return NO_SIGNALS;
	}

	template <size_t SIG_COUNT>
	inline void SignalGroup<SIG_COUNT>::wait(size_t signalIndex) {
		if (unlikely(signalIndex >= SIG_COUNT))
			throw RUNTIME_ERROR("signalIndex out of range");

		// Fast path
		if (tryConsume(signalIndex))
			return;

		std::unique_lock<std::mutex> lock(mtx);
		for (;;) {
			if (tryConsume(signalIndex))
				break;
			eventFired.wait(lock);
		}
	}

	template <size_t SIG_COUNT>
	inline bool SignalGroup<SIG_COUNT>::wait(size_t signalIndex, uint32_t timeoutMs) {
		if (unlikely(signalIndex >= SIG_COUNT))
			throw RUNTIME_ERROR("signalIndex out of range");

		// Fast path
		if (tryConsume(signalIndex))
			return true;

		if (timeoutMs == 0u)
			return false;

		auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);

		std::unique_lock<std::mutex> lock(mtx);
		for (;;) {
			if (tryConsume(signalIndex))
				return true;

			if (eventFired.wait_until(lock, deadline) == std::cv_status::timeout) {
				// One last check before returning timeout
				return tryConsume(signalIndex);
			}
		}
	}

	template <size_t SIG_COUNT>
	inline size_t SignalGroup<SIG_COUNT>::waitOne() {
		// Fast path
		size_t signalIndex = tryConsumeOne();
		if (signalIndex != NO_SIGNALS)
			return signalIndex;

		std::unique_lock<std::mutex> lock(mtx);
		for (;;) {
			signalIndex = tryConsumeOne();
			if (signalIndex != NO_SIGNALS)
				return signalIndex;
			eventFired.wait(lock);
		}
	}

	template <size_t SIG_COUNT>
	inline size_t SignalGroup<SIG_COUNT>::waitOne(uint32_t timeoutMs) {
		// Fast path
		size_t signalIndex = tryConsumeOne();
		if (signalIndex != NO_SIGNALS)
			return signalIndex;

		if (timeoutMs == 0u)
			return NO_SIGNALS;

		auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);

		std::unique_lock<std::mutex> lock(mtx);
		for (;;) {
			signalIndex = tryConsumeOne();
			if (signalIndex != NO_SIGNALS)
				return signalIndex;

			if (eventFired.wait_until(lock, deadline) == std::cv_status::timeout) {
				// One last check before returning timeout
				return tryConsumeOne();
			}
		}
	}

	template <size_t SIG_COUNT>
	inline void SignalGroup<SIG_COUNT>::signal(size_t signalIndex) {
		if (unlikely(signalIndex >= SIG_COUNT))
			throw RUNTIME_ERROR("signalIndex out of range");

		events[signalIndex].store(1u);
		{
			// notify_all (and not notify_one) is mandatory here: the waiters sharing this
			// condition variable may be waiting for different signals, so waking up a single
			// arbitrary one of them could leave the actual target of this signal asleep.
			std::unique_lock<std::mutex> lock(mtx);
			eventFired.notify_all();
		}
	}


	template <size_t SIG_COUNT>
	inline SignalOfAGroup<SIG_COUNT> SignalGroup<SIG_COUNT>::getSignal(size_t signalIndex) {
		if (unlikely(signalIndex >= SIG_COUNT))
			throw RUNTIME_ERROR("signalIndex out of range");

		return SignalOfAGroup<SIG_COUNT>(*this, signalIndex);
	}

	template <size_t SIG_COUNT>
	template <size_t SIGNAL_INDEX>
	inline constexpr SignalOfAGroup<SIG_COUNT> SignalGroup<SIG_COUNT>::getSignal() noexcept {
		static_assert(SIGNAL_INDEX < SIG_COUNT, "signalIndex out of range");
		return SignalOfAGroup<SIG_COUNT>(*this, SIGNAL_INDEX);
	}


	/*
	 * ===== SignalOfAGroup inline implementation =====
	 */

	template <size_t SIG_COUNT>
	inline constexpr SignalOfAGroup<SIG_COUNT>::SignalOfAGroup(SignalGroup<SIG_COUNT>& group_, const size_t signalIndex_) noexcept :
		group(&group_), signalIndex(signalIndex_) {}

	template <size_t SIG_COUNT>
	inline void SignalOfAGroup<SIG_COUNT>::wait() {
		group->wait(signalIndex);
	}

	template <size_t SIG_COUNT>
	inline bool SignalOfAGroup<SIG_COUNT>::wait(uint32_t timeoutMs) {
		return group->wait(signalIndex, timeoutMs);
	}

	template <size_t SIG_COUNT>
	inline void SignalOfAGroup<SIG_COUNT>::signal() {
		group->signal(signalIndex);
	}

};


#endif
