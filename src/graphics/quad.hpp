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
		NONE,
	};

	class Quad
	{
	private:
		std::vector<Vertex>* m_vertices;
		physics::Transform* m_transform;
		Texture* m_texture;

		Mesh* m_mesh = NULL;

		Quad(std::vector<Vertex>* vertices, physics::Transform* transform, Texture* texture);
		static std::vector<Vertex>* createVertices();
		static std::vector<Vertex>* createBlockQuadVertices(world::Block block, Direction direction, AtlasTexture texture);


	public:
		~Quad();
		Mesh* getMesh();
		std::vector<Vertex>* getVertices();

		static Quad* createQuad(physics::Transform* transform, Texture* texture);
		static Quad* createBlockQuad(world::Block block, Direction direction, AtlasTexture texture);


		static const int vertexPositions[];
		static const std::vector<GLuint> indices;
		static const int uvs[];
		static const std::map<Direction, glm::vec3> normalMap;
		static const std::map<Direction, int> vertexPositionIndexMap;
		static const std::map<Direction, int> uvIndexMap;
		static const int vertexCount;
	};

	std::string getName(Direction direction);
	glm::vec3 getNormal(Direction direction);
	world::BlockPos getNormalI(Direction direction);
}