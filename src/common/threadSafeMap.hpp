#pragma once

#include <unordered_map>
#include <mutex>
#include <optional>

template <typename K, typename V>
class ThreadSafeMap
{
public:
	ThreadSafeMap() = default;
	~ThreadSafeMap() = default;

	// Disable copy/move
	ThreadSafeMap(const ThreadSafeMap &) = delete;
	ThreadSafeMap &operator=(const ThreadSafeMap &) = delete;

	bool contains(const K &key) const
	{
		std::scoped_lock lock(m_mutex);
		return m_map.contains(key);
	}

	std::optional<V> get(const K &key) const
	{
		std::scoped_lock lock(m_mutex);
		auto it = m_map.find(key);
		if (it != m_map.end())
		{
			return it->second;
		}
		return std::nullopt;
	}

	// Safe set/insert
	void set(const K &key, const V &value)
	{
		std::scoped_lock lock(m_mutex);
		m_map[key] = value;
	}

	// Safe access with operator[] (for nested maps, etc.)
	V &operator[](const K &key)
	{
		std::scoped_lock lock(m_mutex);
		return m_map[key];
	}

	// Call a lambda with locked access to internal map
	template<typename Func>
	decltype(auto) withLock(Func&& func) {
		std::scoped_lock lock(m_mutex);
		return func(m_map);
	}

private:
	mutable std::mutex m_mutex;
	std::unordered_map<K, V> m_map;
};