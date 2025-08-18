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

	float lerp(float a, float b, float t)
	{
		return a * (1 - t) + b * t;
	}

	float lerp(const glm::vec2& a, const glm::vec2& b, float x)
	{
		float t = (x - a.x) / (b.x - a.x);
		return lerp(a.y, b.y, t);
	}

	float cubicInterpolate(float y0, float y1, float y2, float y3, float mu)
	{
		float a0, a1, a2, a3, mu2;

		mu2 = mu * mu;
		a0 = y3 - y2 - y0 + y1;
		a1 = y0 - y1 - a0;
		a2 = y2 - y0;
		a3 = y1;

		return (a0 * mu * mu2 + a1 * mu2 + a2 * mu + a3);
	}

	float sigmoid(float maxVal, float midPoint, float steepness, float x)
	{
		return maxVal / (1 + std::exp(-steepness * (x - midPoint)));
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

	float evaluateCubic(const glm::vec2* points, int pointCount, float x)
	{
		for (int i = 1; i < pointCount - 2; i++)
		{
			if (x < points[i].x)
			{
				float mu = (x - points[i - 1].x) / (points[i].x - points[i - 1].x);
				return cubicInterpolate(points[i - 2].y, points[i - 1].y, points[i].y, points[i + 1].y, mu);
			}
		}
		throw std::invalid_argument("Point does not fall on given function");
	}

	float roundToNearestNth(float val, float n)
	{
		return std::round(val * n) / n;
	}

	float smoothStep(float edge0, float edge1, float x)
	{
		x = glm::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
		return x * x * (3.0f - 2.0f * x);
	}

	float bilerp(float nw, float ne, float sw, float se, float tx, float tz)
	{
		float top = glm::mix(nw, ne, tx);
		float bottom = glm::mix(sw, se, tx);
		return glm::mix(top, bottom, tz);
	}

	float average(std::vector<float> vec)
	{
		return std::reduce(vec.begin(), vec.end()) / vec.size();
	}
}