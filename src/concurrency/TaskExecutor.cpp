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
				taskToRun = task;
				taskAvailable = false;  // Reset task availability
			}
			if (taskToRun) {
				taskToRun();  // Execute the task
				{
					std::unique_lock<std::mutex> lock(mtx);
					taskDone = true;  // Mark task as done
				}
				cvTaskDone.notify_one();  // Notify that task is done
			}
		}
	}

	TaskExecutor::TaskExecutor() :
		taskAvailable(false),
		stopFlag(false),
		taskDone(true)
	{
		worker = std::thread(&TaskExecutor::workerThreadBody, this);  // Launch worker thread
	}

	TaskExecutor::~TaskExecutor() {
		{
			std::unique_lock<std::mutex> lock(mtx);
			stopFlag = true;
			taskAvailable = true;  // Wake up the thread if it's waiting
		}
		cvTaskAvailable.notify_all();
		worker.join();  // Ensure the worker thread is properly joined
	}

	void TaskExecutor::waitForTaskCompletion() {
		std::unique_lock<std::mutex> lock(mtx);
		cvTaskDone.wait(lock, [&] { return taskDone; });  // Wait for task completion
	}

	void TaskExecutor::submitVoidTask(const std::function<void()>& newTask) {
		{
			std::unique_lock<std::mutex> lock(mtx);
			task = newTask;
			taskAvailable = true;
			taskDone = false;
		}
		cvTaskAvailable.notify_one();  // Notify the worker thread
	}

};