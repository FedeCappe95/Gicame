#include "concurrency/TaskExecutor.hpp"


namespace Gicame::Concurrency {

	// Function run by the worker thread
	void TaskExecutor::workerThreadBody() {
		while (true) {
			std::function<void()> taskToRun;
			{
				std::unique_lock<std::mutex> lock(mtx);
				cvTaskAvailable.wait(lock, [&] { return taskAvailable || stopFlag; });  // Wait for task or stop signal
				if (stopFlag)
					return;  // Exit if stop is signaled
				taskToRun.swap(task);
				taskAvailable = false;  // Reset task availability
				taskCompletionState = CompletionState::NOT_DONE;  // Could be not necessary
				lastTaskException.reset();
			}
			if (taskToRun) {
				bool success = false;
				try {
					taskToRun();  // Execute the task
					success = true;
				}
				catch (const std::exception& exc) {
					std::unique_lock<std::mutex> lock(mtx);
					taskCompletionState = CompletionState::DONE_WITH_ERRORS;  // Mark task as done
					lastTaskException = exc;
				}
				catch (...) {
					std::unique_lock<std::mutex> lock(mtx);
					taskCompletionState = CompletionState::DONE_WITH_ERRORS;  // Mark task as done
				}
				if (success) {
					std::unique_lock<std::mutex> lock(mtx);
					taskCompletionState = CompletionState::DONE;  // Mark task as done
				}
				cvTaskDone.notify_one();  // Notify that task is done
			}
		}
	}

	TaskExecutor::TaskExecutor() :
		taskAvailable(false),
		stopFlag(false),
		taskCompletionState(CompletionState::NOT_DONE)
	{
		worker = std::thread(&TaskExecutor::workerThreadBody, this);  // Launch worker thread
	}

	TaskExecutor::~TaskExecutor() {
		{
			std::unique_lock<std::mutex> lock(mtx);
			stopFlag = true;  // It also wakes up the thread if it's waiting
		}
		cvTaskAvailable.notify_all();
		worker.join();  // Ensure the worker thread is properly joined
	}

	TaskExecutor::CompletionState TaskExecutor::waitForTaskCompletion() {
		std::unique_lock<std::mutex> lock(mtx);
		cvTaskDone.wait(lock, [&] { return taskCompletionState != CompletionState::NOT_DONE; });
		return taskCompletionState;
	}

	void TaskExecutor::submitVoidTask(const std::function<void()>& newTask) {
		{
			std::unique_lock<std::mutex> lock(mtx);
			cvTaskAvailable.wait(lock, [&] { return !taskAvailable; });
			task = newTask;
			taskAvailable = true;
			taskCompletionState = CompletionState::NOT_DONE;
		}
		cvTaskAvailable.notify_one();  // Notify the worker thread
	}

	std::optional<std::exception> TaskExecutor::getLastTaskException() {
		return lastTaskException;
	}

};