#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <functional>

class ThreadPool {
	std::queue<std::function<void()>> jobs;
	std::mutex queueMutex;
	std::atomic<int> wake = 0;
	std::atomic<long> jobCount = 0;



public:
	std::vector<std::jthread> threads;

	ThreadPool() {
		threads.reserve(1);
		threads.emplace_back([this](std::stop_token st) {
			this->worker(st);
			});
	}
	ThreadPool(int numThreads) {
		threads.reserve(numThreads);
		for (int i = 0; i < numThreads; i++)
			threads.emplace_back([this](std::stop_token st) {
				this->worker(st);
			});
	}
	~ThreadPool() {
		for (auto& t : threads)
			t.request_stop();
		wake++;
		wake.notify_all();
	}

	void worker(std::stop_token st) {
		while (!st.stop_requested()) {
			bool gotJob = false;
			std::function<void()> job;

			//lock and check if work available
			{
				std::lock_guard<std::mutex> lock(queueMutex);
				if (!jobs.empty()) {
					job = std::move(jobs.front());
					jobs.pop();
					gotJob = true;
				}
			}

			//execut job
			if (gotJob) {
				job();
				jobCount--;
				wake++;
				wake.notify_all();
				continue;
			}

			//wait for work
			auto w = wake.load(std::memory_order_acquire);
			wake.wait(w, std::memory_order_relaxed);
		}
	}

	//adding jobs
	void enqueue(std::function<void()> job) {
		//add job to queue whilst locked
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			jobs.push(job);
			jobCount++;
		}
		wake++;
		wake.notify_one();
	}
	
	void waitForCompletion() {
		while (true) {
			if (jobCount == 0 && jobs.empty())
				return;

			std::this_thread::yield();  // Busy wait instead of atomic wait
		}
	}

};