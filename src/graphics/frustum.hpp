#pragma once

#include <array>
#include <glm/geometric.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include "camera.hpp"
#include "../world/chunk.hpp"

namespace voxel_game::graphics
{
	struct Plane
	{
		glm::vec3 normal;
		float distance;
	};

	glm::mat4 perspectiveProjectionMatrix();
	std::array<Plane, 6> buildFrustum(const glm::mat4& viewProjection);
	std::array<Plane, 6> buildCameraFrustum(Camera* camera);
	bool aabbIntersectsFrustum(const glm::vec3& min, const glm::vec3& max, const std::array<Plane, 6>& frustum);
	bool chunkIntersectsFrustum(world::Chunk* chunk, const std::array<Plane, 6>& frustum);
}
