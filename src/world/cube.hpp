#pragma once

#include <vector>
#include "block.hpp"
#include "../graphics/quad.hpp"

namespace g = voxel_game::graphics;

namespace voxel_game::world
{
	class Cube
	{
	private:
		std::vector<g::Quad*> m_faces;

	public:
		Cube(std::vector<g::Quad*> faces);
		~Cube();
		g::Quad* getFace(g::Direction);
	};

	Cube singleTextureCube(world::Block block, g::AtlasTexture tex);
	Cube multiTextureCube(world::Block block,
		g::AtlasTexture frontTex,
		g::AtlasTexture backTex,
		g::AtlasTexture leftTex,
		g::AtlasTexture rightTex,
		g::AtlasTexture topTex,
		g::AtlasTexture bottomTex);

	class BlockType
	{
	private:
		BlockTypeId m_id;
		std::function<Cube(Block block)> m_meshConstructor;

	public:
		BlockType(BlockTypeId id, std::function<Cube(Block block)> constructor) : m_id(id), m_meshConstructor(constructor) {}
		inline std::function<Cube(Block block)> getMeshConstructor() const { return m_meshConstructor; }

		inline bool operator==(const BlockType& other) const
		{
			return m_id == other.m_id;
		}

		static const BlockType& get(BlockTypeId id)
		{
			static const std::map<BlockTypeId, BlockType> blockTypes = {
				{BlockTypeId::NONE, BlockType(BlockTypeId::NONE, nullptr)},
				{BlockTypeId::AIR, BlockType(BlockTypeId::AIR, nullptr)},
				{BlockTypeId::STONE, BlockType(BlockTypeId::STONE, [](Block block) { return singleTextureCube(block, g::AtlasTexture::STONE); })},
				{BlockTypeId::DIRT, BlockType(BlockTypeId::DIRT, [](Block block) { return singleTextureCube(block, g::AtlasTexture::DIRT); })},
				{BlockTypeId::GRASS, BlockType(BlockTypeId::GRASS, [](Block block) { return multiTextureCube(block, g::AtlasTexture::GRASS_SIDE, g::AtlasTexture::GRASS_SIDE, g::AtlasTexture::GRASS_SIDE, g::AtlasTexture::GRASS_SIDE, g::AtlasTexture::GRASS, g::AtlasTexture::DIRT); })},
				{BlockTypeId::BEDROCK, BlockType(BlockTypeId::BEDROCK, [](Block block) { return singleTextureCube(block, g::AtlasTexture::BEDROCK); })} };
			return blockTypes.at(id);
		}
	};

	inline bool isSolid(BlockTypeId blockType) { return blockType != BlockTypeId::NONE && blockType != BlockTypeId::AIR; }
}