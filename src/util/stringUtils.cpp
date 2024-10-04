#include "stringUtils.hpp"

namespace voxel_game::utils
{
	std::string formatF(float f, int precision)
	{
		std::stringstream stream;
		stream << std::fixed << std::setprecision(precision) << f;
		return stream.str();
	}
}