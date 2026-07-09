#include "frustum.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace voxel_game::graphics
{
	namespace
	{
		Plane normalizePlane(Plane plane)
		{
			float length = glm::length(plane.normal);
			return Plane{ plane.normal / length, plane.distance / length };
		}

		Plane makePlane(const glm::mat4& mat, int rowSign, int row)
		{
			return normalizePlane(Plane{
				glm::vec3(
					mat[0][3] + rowSign * mat[0][row],
					mat[1][3] + rowSign * mat[1][row],
					mat[2][3] + rowSign * mat[2][row]
				),
				mat[3][3] + rowSign * mat[3][row]
			});
		}
	}

	glm::mat4 perspectiveProjectionMatrix()
	{
		static const glm::mat4 projection = glm::perspective(70.f, 16.f / 9, 0.1f, 1000.f);
		return projection;
	}

	std::array<Plane, 6> buildFrustum(const glm::mat4& viewProjection)
	{
		return {
			makePlane(viewProjection, 1, 0),
			makePlane(viewProjection, -1, 0),
			makePlane(viewProjection, 1, 1),
			makePlane(viewProjection, -1, 1),
			makePlane(viewProjection, 1, 2),
			makePlane(viewProjection, -1, 2),
		};
	}

	std::array<Plane, 6> buildCameraFrustum(Camera* camera)
	{
		return buildFrustum(perspectiveProjectionMatrix() * camera->viewMatrix());
	}

	bool aabbIntersectsFrustum(const glm::vec3& min, const glm::vec3& max, const std::array<Plane, 6>& frustum)
	{
		for (const Plane& plane : frustum)
		{
			glm::vec3 positiveVertex = min;
			if (plane.normal.x >= 0)
			{
				positiveVertex.x = max.x;
			}
			if (plane.normal.y >= 0)
			{
				positiveVertex.y = max.y;
			}
			if (plane.normal.z >= 0)
			{
				positiveVertex.z = max.z;
			}

			if (glm::dot(plane.normal, positiveVertex) + plane.distance < 0)
			{
				return false;
			}
		}

		return true;
	}

	bool chunkIntersectsFrustum(world::Chunk* chunk, const std::array<Plane, 6>& frustum)
	{
		world::BlockPos origin = chunk->getOrigin();
		glm::vec3 min(origin.x, origin.y, origin.z);
		glm::vec3 max(origin.x + CHUNK_SIZE, origin.y + CHUNK_HEIGHT, origin.z + CHUNK_SIZE);
		return aabbIntersectsFrustum(min, max, frustum);
	}
}
