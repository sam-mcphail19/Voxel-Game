#pragma once

#include <string>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <sstream>

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
		static inline std::string buildMessage(const T &message, const LogLevel &level)
		{
			std::ostringstream oss;
			oss << currentTimestamp()
				<< " [" << level << "] "
				<< "(Thread-" << std::this_thread::get_id() << ") "
				<< message;
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