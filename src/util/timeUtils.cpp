#include "timeUtils.hpp"

namespace voxel_game::utils
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	float getElapsedTime()
	{
		auto currentTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> elapsedTime = currentTime - startTime;
		return elapsedTime.count();
	}
}