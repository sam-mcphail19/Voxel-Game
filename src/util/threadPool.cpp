#include "threadPool.hpp"

namespace voxel_game::utils
{
	void ThreadPool::start() {
		const uint32_t num_threads = std::thread::hardware_concurrency(); // Max # of threads the system supports
		for (uint32_t i = 0; i < num_threads; i++) {
			m_threads.emplace_back(std::thread(&ThreadPool::threadLoop, this));
		}
	}

	void ThreadPool::threadLoop() {
		while (true) {
			std::function<void()> job;
			{
				std::unique_lock<std::mutex> lock(m_queueMutex);
				m_mutexCondition.wait(lock, [this] {
					return !m_jobs.empty() || m_shouldTerminate;
					});
				if (m_shouldTerminate) {
					return;
				}
				job = m_jobs.front();
				m_jobs.pop();
			}
			job();
		}
	}

	void ThreadPool::queueJob(const std::function<void()>& job) {
		{
			std::unique_lock<std::mutex> lock(m_queueMutex);
			m_jobs.push(job);
		}

		m_mutexCondition.notify_one();
	}

	bool ThreadPool::busy() {
		bool poolbusy;
		{
			std::unique_lock<std::mutex> lock(m_queueMutex);
			poolbusy = !m_jobs.empty();
		}
		return poolbusy;
	}

	void ThreadPool::stop() {
		{
			std::unique_lock<std::mutex> lock(m_queueMutex);
			m_shouldTerminate = true;
		}
		m_mutexCondition.notify_all();
		for (std::thread& active_thread : m_threads) {
			active_thread.join();
		}
		m_threads.clear();
	}
}