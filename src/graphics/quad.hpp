#pragma once

#include <map>
#include <string>
#include <vector>
#include <glm/vec3.hpp>
#include "atlasTexture.hpp"
#include "textureAtlas.hpp"
#include "mesh.hpp"
#include "../world/block.hpp"

namespace voxel_game::graphics
{
	enum class Direction
	{
		FRONT,
		BACK,
		RIGHT,
		LEFT,
		TOP,
		BOTTOM,
	};

	const int vertexCount = 6;

	class Quad: public Mesh
	{
	private:
		static std::vector<Vertex>* createVertices();
		static std::vector<Vertex>* createBlockQuadVertices(world::Block block, Direction direction, AtlasTexture texture);

	public:
		using Mesh::Mesh;
		static Quad* createQuad(physics::Transform *transform, Texture *texture);
		static Quad* createBlockQuad(world::Block block, physics::Transform *transform, Direction direction, AtlasTexture texture);
	};
	
	glm::vec3 getNormal(Direction direction);
}