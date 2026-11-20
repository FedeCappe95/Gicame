#ifndef __GICAME__CONCURRENCY__INTERPROCESSMUTEX_H__
#define __GICAME__CONCURRENCY__INTERPROCESSMUTEX_H__


#include <string>
#include <chrono>
#include "../common.h"
#include "../configuration.h"
#include "../utils/NotCopyable.h"
#ifndef WINDOWS
#include "../sm/SharedMemory.h"
#endif


namespace Gicame::Concurrency::Impl { struct RobustMutexData; };


namespace Gicame::Concurrency {

	/**
	 * @brief System level named robust mutex
	 *
	 * When its owner dies while holding it, the next acquisition gets it anyway and reports
	 * OWNER_DIED: the data it protects may have been left half-updated.
	 * The mutex is recursive and owned by the acquiring thread, which must release it once per
	 * acquisition. It satisfies the Lockable requirements.
	 */
	class InterprocessMutex {

		NOT_COPYABLE(InterprocessMutex)

	public:
		enum class AcquireResult {
			SUCCESS,        // Lock acquired successfully
			OWNER_DIED,     // Lock acquired, but its previous owner died while holding it
			TIMEOUT,        // Timeout expired before acquiring lock
			FAIL            // An error occurred
		};

	private:
#ifdef WINDOWS
		void* handle;
#else
		Gicame::SharedMemory shmem;
		Impl::RobustMutexData* sharedData;
#endif
		bool previousOwnerDead;

	private:
		AcquireResult recordAcquisition(const AcquireResult result) noexcept;

	public:
		/**
		 * @param name The name of the mutex (see Os::isValidObjectName)
		 */
		GICAME_API InterprocessMutex(const std::string& name);

		GICAME_API ~InterprocessMutex();

		GICAME_API AcquireResult acquire();

		GICAME_API AcquireResult acquireWithTimeout(const std::chrono::milliseconds timeout);

		/**
		 * @brief Release the mutex (only the owner thread can)
		 * @return true if successful, false on error
		 */
		GICAME_API bool release();

		/**
		 * @brief Lock the mutex (throws on error) - for std::lock_guard compatibility
		 *
		 * OWNER_DIED counts as acquired: isPreviousOwnerDead() tells it apart.
		 */
		GICAME_API void lock();

		/**
		 * @brief Unlock the mutex (throws on error) - for std::lock_guard compatibility
		 */
		GICAME_API void unlock();

		/**
		 * @brief Try to lock without blocking
		 * @return true if acquired, also when the previous owner died (see isPreviousOwnerDead())
		 */
		GICAME_API bool try_lock();

		/**
		 * @brief Whether the owner before the calling thread died while holding the mutex
		 *
		 * Valid while the calling thread holds the mutex, about the acquisition that gave it, e.g.
		 * right after lock(). A nested acquisition of the recursive mutex resets it to false.
		 */
		GICAME_API bool isPreviousOwnerDead() const noexcept;

		/**
		 * @brief Set whether the mutex name should be removed on destruction
		 * @param uod If true, remove it on destruction (default: true only for the instance that
		 *            initialized the mutex; ignored on Windows)
		 */
		GICAME_API void setUnlinkOnDestruction(bool uod);

	};

};


#endif
