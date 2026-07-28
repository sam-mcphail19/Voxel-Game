#pragma once

#include <array>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include "debugInfo.hpp"

namespace voxel_game::world
{
	class WorldProfiler
	{
	private:
		static constexpr size_t STAGE_COUNT = static_cast<size_t>(ProfileStage::Count);
		static constexpr size_t COUNTER_COUNT = static_cast<size_t>(ProfileCounter::Count);
		static constexpr size_t LATENCY_BUCKET_COUNT = 32;
		std::array<std::atomic<uint64_t>, STAGE_COUNT> m_totalNanoseconds{};
		std::array<std::atomic<uint64_t>, STAGE_COUNT> m_callCounts{};
		std::array<std::atomic<uint64_t>, STAGE_COUNT> m_maximumNanoseconds{};
		std::array<std::array<std::atomic<uint64_t>, LATENCY_BUCKET_COUNT>, STAGE_COUNT> m_latencyBuckets{};
		std::array<std::atomic<uint64_t>, COUNTER_COUNT> m_counters{};

		static std::array<uint64_t, COUNTER_COUNT>& localCounters()
		{
			static thread_local std::array<uint64_t, COUNTER_COUNT> counters{};
			return counters;
		}

	public:
		static WorldProfiler& instance()
		{
			static WorldProfiler profiler;
			return profiler;
		}

		void record(ProfileStage stage, uint64_t nanoseconds)
		{
			const size_t index = static_cast<size_t>(stage);
			m_totalNanoseconds[index].fetch_add(nanoseconds, std::memory_order_relaxed);
			m_callCounts[index].fetch_add(1, std::memory_order_relaxed);
			uint64_t currentMaximum = m_maximumNanoseconds[index].load(std::memory_order_relaxed);
			while (currentMaximum < nanoseconds
				&& !m_maximumNanoseconds[index].compare_exchange_weak(
					currentMaximum, nanoseconds, std::memory_order_relaxed))
			{
			}

			uint64_t microseconds = std::max<uint64_t>(1, nanoseconds / 1'000);
			size_t bucket = 0;
			while (microseconds > 1 && bucket + 1 < LATENCY_BUCKET_COUNT)
			{
				microseconds >>= 1;
				bucket++;
			}
			m_latencyBuckets[index][bucket].fetch_add(1, std::memory_order_relaxed);
		}

		void increment(ProfileCounter counter, uint64_t amount = 1)
		{
			localCounters()[static_cast<size_t>(counter)] += amount;
		}

		void flushLocalCounters()
		{
			auto& local = localCounters();
			for (size_t index = 0; index < COUNTER_COUNT; ++index)
			{
				if (local[index] == 0)
				{
					continue;
				}
				m_counters[index].fetch_add(local[index], std::memory_order_relaxed);
				local[index] = 0;
			}
		}

		GenerationDiagnostics takeSnapshot()
		{
			flushLocalCounters();
			GenerationDiagnostics result;
			for (size_t index = 0; index < STAGE_COUNT; ++index)
			{
				result.stages[index].totalNanoseconds =
					m_totalNanoseconds[index].exchange(0, std::memory_order_relaxed);
				result.stages[index].callCount =
					m_callCounts[index].exchange(0, std::memory_order_relaxed);
				result.stages[index].maximumNanoseconds =
					m_maximumNanoseconds[index].exchange(0, std::memory_order_relaxed);

				const uint64_t percentileTarget =
					(result.stages[index].callCount * 95 + 99) / 100;
				uint64_t samples = 0;
				for (size_t bucket = 0; bucket < LATENCY_BUCKET_COUNT; ++bucket)
				{
					samples += m_latencyBuckets[index][bucket].exchange(
						0, std::memory_order_relaxed);
					if (percentileTarget > 0 && samples >= percentileTarget
						&& result.stages[index].p95Nanoseconds == 0)
					{
						result.stages[index].p95Nanoseconds =
							(1ULL << std::min<size_t>(bucket + 1, 31)) * 1'000;
					}
				}
				result.stages[index].p95Nanoseconds = std::min(
					result.stages[index].p95Nanoseconds,
					result.stages[index].maximumNanoseconds);
			}
			for (size_t index = 0; index < COUNTER_COUNT; ++index)
			{
				result.counters[index] = m_counters[index].exchange(0, std::memory_order_relaxed);
			}
			return result;
		}
	};

	class ScopedProfileStage
	{
	private:
		ProfileStage m_stage;
		std::chrono::steady_clock::time_point m_start = std::chrono::steady_clock::now();

	public:
		explicit ScopedProfileStage(ProfileStage stage) : m_stage(stage) {}

		~ScopedProfileStage()
		{
			const auto elapsed = std::chrono::steady_clock::now() - m_start;
			WorldProfiler::instance().record(
				m_stage,
				static_cast<uint64_t>(
					std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count()));
		}
	};

	class ScopedProfileCounterBatch
	{
	public:
		~ScopedProfileCounterBatch()
		{
			WorldProfiler::instance().flushLocalCounters();
		}
	};
}
