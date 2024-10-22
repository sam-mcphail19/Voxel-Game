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
		// If a and b have opposite signs and there's a remainder, subtract 1 from the result
		if ((a % b != 0) && ((a < 0) != (b < 0))) {
			result--;
		}
		return result;
	}
	
	float lerp(const glm::vec2& a, const glm::vec2& b, float x)
	{
		float t = (x - a.x) / (b.x - a.x);
		return glm::mix(a.y, b.y, t);
	}

	float evaluate(const glm::vec2* points, int pointCount, float x)
	{
		for (int i = 1; i < pointCount; i++)
		{
			if (x < points[i].x)
			{
				return lerp(points[i - 1], points[i], x);
			}
		}

		throw std::invalid_argument("Point does not fall on given function");
	}

	float roundToNearestNth(float val, float n)
	{
		return std::round(val * n) / n;
	}
}