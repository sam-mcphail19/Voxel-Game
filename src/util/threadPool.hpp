#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace voxel_game::utils
{
	struct ThreadPoolStats
	{
		size_t activeJobs;
		size_t queuedJobs;
	};

	class ThreadPool {
	public:
		void start();
		void queueJob(const std::function<void()>& job);
		void stop();
		bool busy();
		ThreadPoolStats getStats();

	private:
		void threadLoop();

		bool m_shouldTerminate = false;
		std::mutex m_queueMutex;
		std::condition_variable m_mutexCondition;
		std::vector<std::thread> m_threads;
		std::queue<std::function<void()>> m_jobs;
		size_t m_activeJobs = 0;
	};
}
