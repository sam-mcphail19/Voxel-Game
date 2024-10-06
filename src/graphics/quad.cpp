#include "quad.hpp"

namespace voxel_game::graphics
{
	int vertexPositions[] = {
        // Front Face
        0, 1, 1,
        0, 0, 1,
        1, 0, 1,
        1, 1, 1,

		// Back Face
        1, 0, 0,
        0, 0, 0,
        0, 1, 0,
        1, 1, 0,

        // Right Face
        1, 1, 1,
        1, 0, 1,
        1, 0, 0,
        1, 1, 0,

        // Left Face
        0, 1, 1,
        0, 1, 0,
        0, 0, 0,
        0, 0, 1,

		// Top Face
        0, 1, 0,
        0, 1, 1,
        1, 1, 1,
        1, 1, 0,

        // Bottom Face
        1, 0, 0,
        1, 0, 1,
        0, 0, 1,
        0, 0, 0,
    };

    std::vector<GLuint> indices ({0, 1, 3, 3, 1, 2});

    int uvs[] = {
        // Front Face
        0, 1,
        0, 0,
        1, 0,
        1, 1,

		// Back Face
        1, 0,
        0, 0,
        0, 1,
        1, 1,

        // Right Face
        0, 1,
        0, 0,
        1, 0,
        1, 1,

        // Left Face
        1, 1,
        0, 1,
        0, 0,
        1, 0,

		// Top Face
        0, 0,
        1, 0,
        1, 1,
        0, 1,

        // Bottom Face
        0, 0,
        0, 1,
        1, 1,
        1, 0,  
    };

	std::map<Direction, glm::vec3> normalMap = {
		{Direction::FRONT, glm::vec3(0, 0, 1)},
		{Direction::BACK, glm::vec3(0, 0, -1)},
		{Direction::RIGHT, glm::vec3(1, 0, 0)},
		{Direction::LEFT, glm::vec3(-1, 0, 0)},
		{Direction::TOP, glm::vec3(0, 1, 0)},
		{Direction::BOTTOM, glm::vec3(0, -1, 0)},
	};

	std::map<Direction, int> vertexPositionIndexMap = {
		{Direction::FRONT, 0},
		{Direction::BACK, 12},
		{Direction::RIGHT, 24},
		{Direction::LEFT, 36},
		{Direction::TOP, 48},
		{Direction::BOTTOM, 60},
	};

	std::map<Direction, int> uvIndexMap = {
		{Direction::FRONT, 0},
		{Direction::BACK, 8},
		{Direction::RIGHT, 16},
		{Direction::LEFT, 24},
		{Direction::TOP, 32},
		{Direction::BOTTOM, 40},
	};

	Quad *Quad::createQuad(physics::Transform transform, Texture *texture)
	{
		return new Quad(*createVertices(), indices, transform, texture);
	}

	Quad *Quad::createBlockQuad(world::Block block, physics::Transform transform, Direction direction, AtlasTexture texture)
	{
		return new Quad(*createBlockQuadVertices(block, direction, texture), indices, transform, loadTextureAtlas());
	}

	glm::vec3 getNormal(Direction direction)
	{
		return normalMap[direction];
	}

	// TODO: this probably shouldnt be a vector
	std::vector<Vertex>* Quad::createVertices()
	{
		std::vector<Vertex>* vertices = new std::vector<Vertex>;

		int startingIndex = vertexPositionIndexMap[Direction::FRONT];
		int uvIndex = uvIndexMap[Direction::FRONT];

		for (int i = 0; i < vertexCount; i++)
		{
			glm::vec3 pos = glm::vec3(
				vertexPositions[startingIndex + i * 3],
				vertexPositions[startingIndex + i * 3 + 1],
				vertexPositions[startingIndex + i * 3 + 2]);
			glm::vec2 uv = glm::vec2(uvs[uvIndex + i * 2], uvs[uvIndex + i * 2 + 1]);
			Vertex vert(pos, normalMap[Direction::FRONT], uv, world::BlockTypeId::NONE, {0,0,0}, false);
			vertices->push_back(vert);
		}

		return vertices;
	}

	// TODO: this probably shouldnt be a vector
	std::vector<Vertex>* Quad::createBlockQuadVertices(world::Block block, Direction direction, AtlasTexture texture)
	{
		std::vector<Vertex>* vertices = new std::vector<Vertex>;

		int startingIndex = vertexPositionIndexMap[direction];
		int uvIndex = uvIndexMap[direction];

		for (int i = 0; i < vertexCount; i++)
		{
			glm::vec3 pos = glm::vec3(
				vertexPositions[startingIndex + i * 3],
				vertexPositions[startingIndex + i * 3 + 1],
				vertexPositions[startingIndex + i * 3 + 2]
			);
			glm::vec2 atlasCoords = getTextureAtlasCoords(texture);
			float textureSize = (float)ATLAS_TEXTURE_SIZE / getTextureAtlasSize();
			glm::vec2 uv = glm::vec2(
				uvs[uvIndex + i * 2] * textureSize + atlasCoords.x,
				uvs[uvIndex + i * 2 + 1] * textureSize + atlasCoords.y
			);
			Vertex vert(pos, normalMap[direction], uv, block.type, block.pos, true);
			vertices->push_back(vert);
		}

		return vertices;
	}
}