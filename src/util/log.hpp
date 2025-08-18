#pragma once

#include <string>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <sstream>
#include <type_traits>
#include <iterator>

namespace voxel_game::log
{
	enum class LogLevel
	{
		Info,
		Error
	};

	inline std::ostream &operator<<(std::ostream &os, LogLevel level)
	{
		switch (level)
		{
		case LogLevel::Error:
			return os << "ERROR";
		case LogLevel::Info:
		default:
			return os << "INFO";
		}
	}

	template <typename T, typename = void>
	struct is_iterable : std::false_type {};

	// If calling std::begin(t) and std::end(t) is valid for some type T then is_iterable<T> should inherit from std::true_type
	template <typename T>
	struct is_iterable<T, std::void_t<decltype(std::begin(std::declval<T>())), decltype(std::end(std::declval<T>()))>> : std::true_type {};

	// Exclude std::string from iterable handling
	template <typename T>
	constexpr bool is_iterable_but_not_string =
		is_iterable<T>::value &&
		!std::is_same<std::decay_t<T>, std::string>::value;
	
	class Logger
	{
	private:
		static inline std::string currentTimestamp()
		{
			const auto now = std::chrono::system_clock::now();
			const auto now_time_t = std::chrono::system_clock::to_time_t(now);
			const auto now_local = *std::gmtime(&now_time_t);
			const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;

			std::ostringstream oss;
			oss << "[" << std::put_time(&now_local, "%H:%M:%S")
				<< "." << std::setw(3) << std::setfill('0') << ms << "]";
			return oss.str();
		}

		template <typename T>
		static inline typename std::enable_if<!is_iterable_but_not_string<T>, std::string>::type
		formatMessage(const T &message)
		{
			std::ostringstream oss;
			oss << message;
			return oss.str();
		}

		template <typename T>
		static inline typename std::enable_if<is_iterable_but_not_string<T>, std::string>::type
		formatMessage(const T &message)
		{
			std::ostringstream oss;
			oss << "[";
			for (auto it = std::begin(message); it != std::end(message); ++it)
			{
				if (it != std::begin(message)) oss << ", ";
				oss << *it;
			}
			oss << "]";
			return oss.str();
		}

		template <typename T>
		static inline std::string buildMessage(const T &message, const LogLevel &level)
		{
			std::ostringstream oss;
			oss << currentTimestamp()
				<< " [" << level << "] "
				<< "(Thread-" << std::this_thread::get_id() << ") "
				<< formatMessage(message);
			return oss.str();
		}

	public:
		template <typename T>
		static inline void info(const T &message)
		{
			std::clog << buildMessage(message, LogLevel::Info) << std::endl;
		}

		template <typename T>
		static inline void error(const T &message)
		{
			std::cerr << buildMessage(message, LogLevel::Error) << std::endl;
		}
	};

	template <typename T>
	inline void info(const T &message)
	{
		Logger::info(message);
	}

	template <typename T>
	inline void error(const T &message)
	{
		Logger::error(message);
	}
}