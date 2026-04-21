#ifndef __GICAME__SEMAPHORE_H__
#define __GICAME__SEMAPHORE_H__


#include <string>
#include <stdexcept>
#include <stdint.h>
#include <chrono>
#include "../common.h"
#ifndef WINDOWS
#include <semaphore.h>
#endif



namespace Gicame {

	/**
	 * @brief System level semaphore
	 *
	 * It satisfies the Lockable requirements.
	 */
	class Semaphore {

	public:
		/**
		 * @brief Result of a timeout-aware acquire operation
		 */
		enum class AcquireResult {
			SUCCESS,        // Lock acquired successfully
			TIMEOUT,        // Timeout expired before acquiring lock
			FAIL           // An error occurred
		};

	private:
#ifdef WINDOWS
		void* handle;
#else
		sem_t* handle;
		const std::string posixName;
#endif
		bool unlinkOnDestruction;

	public:
		/**
		 * @brief Constructor
		 * @param name The name of the semaphore (cannot be empty, cannot contain '/' on POSIX)
		 * @param maxConcurrency Maximum number of concurrent acquisitions (cannot be 0)
		 */
		GICAME_API Semaphore(const std::string& name, const size_t maxConcurrency);

		GICAME_API ~Semaphore();

		/**
		 * @brief Acquire the semaphore (blocking, no timeout)
		 * @return true if successful, false on error
		 */
		GICAME_API bool acquire();

		/**
		 * @brief Acquire the semaphore with timeout
		 * @param timeout Maximum time to wait for the lock
		 */
		AcquireResult acquireWithTimeout(const std::chrono::milliseconds timeout);

		/**
		 * @brief Release the semaphore
		 * @return true if successful, false on error
		 */
		GICAME_API bool release();

		/**
		 * @brief Lock the semaphore (throws on error) - for std::lock_guard compatibility
		 */
		GICAME_API void lock();

		/**
         * @brief Unlock the semaphore (throws on error) - for std::lock_guard compatibility
         */
		GICAME_API void unlock();

		/**
		 * @brief Try to lock without blocking
		 * @return true if acquired, false if already locked
		 */
		GICAME_API bool try_lock();

		/**
		 * @brief Set whether the semaphore should be unlinked on destruction
		 * @param uod If true, unlink on destruction (default is true)
		 */
		GICAME_API void setUnlinkOnDestruction(bool uod);

	};

};


#endif
