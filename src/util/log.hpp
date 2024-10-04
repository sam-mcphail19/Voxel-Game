#pragma once

#include <string>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>

namespace voxel_game::log
{
	template <typename T>
	void info(T message)
	{
		const auto currentDateTime = std::chrono::system_clock::now();
		const auto currentDateTimeTimeT = std::chrono::system_clock::to_time_t(currentDateTime);
		const auto currentDateTimeLocalTime = *std::gmtime(&currentDateTimeTimeT);
		const auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(currentDateTime).time_since_epoch().count() % 1000;
		std::ostringstream oss;
		oss << "[" << std::put_time(&currentDateTimeLocalTime, "%H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms << "] ";
		oss << "(Thread-" << std::this_thread::get_id() << ") ";
		oss << message;

		std::clog << oss.str() << std::endl;
	}
}