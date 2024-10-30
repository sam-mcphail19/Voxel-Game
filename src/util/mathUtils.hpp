#pragma once

#include <vector>
#include <stdexcept>
#include <glm/glm.hpp>
#include "../util/log.hpp"

namespace voxel_game::utils
{
	float max(float a, float b);
	float min(float a, float b);
	int max(int a, int b);
	int min(int a, int b);
	int floorDiv(int a, int b);
	float lerp(float a, float b, float t);
	float lerp(const glm::vec2& a, const glm::vec2& b, float x);
	float cubicInterpolate(float y0, float y1, float y2, float y3, float mu);
	// Given an array of points that describe a piece-wise linear function, evaluates the function at the given x
	float evaluate(const glm::vec2* points, int pointCount, float x);
	float evaluateCubic(const glm::vec2* points, int pointCount, float x);

	float roundToNearestNth(float val, float n);
}