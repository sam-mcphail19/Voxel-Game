#include "mathUtils.hpp"

namespace voxel_game::utils
{
	float max(float a, float b)
	{
		return a >= b ? a : b;
	}

	float min(float a, float b)
	{
		return a <= b ? a : b;
	}

	int max(int a, int b)
	{
		return a >= b ? a : b;
	}

	int min(int a, int b)
	{
		return a <= b ? a : b;
	}
	
	int floorDiv(int a, int b)
	{
		int result = a / b;
		// If x and y have opposite signs and there's a remainder, subtract 1 from the result
		if ((a % b != 0) && ((a < 0) != (b < 0))) {
			result--;
		}
		return result;
	}
}