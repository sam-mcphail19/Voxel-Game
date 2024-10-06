#pragma once

#include <map>
#include "block.hpp"
#include "../graphics/quad.hpp"

namespace g = voxel_game::graphics;

namespace voxel_game::world
{
	class Cube
	{
	private:
		std::map<g::Direction, g::Quad *> m_faces;

	public:
		Cube(std::map<g::Direction, g::Quad *> faces);
		std::map<g::Direction, g::Quad *>* getFaces();
	};

	Cube singleTextureCube(world::Block block, physics::Transform transform, g::AtlasTexture tex);
	Cube multiTextureCube(world::Block block, physics::Transform transform,
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
		std::function<Cube(Block block, physics::Transform transform)> m_meshConstructor;

	public:
		BlockType(BlockTypeId id, std::function<Cube(Block block, physics::Transform transform)> constructor): m_id(id), m_meshConstructor(constructor) {}
		inline std::function<Cube(Block block, physics::Transform transform)> getMeshConstructor() const { return m_meshConstructor; }

		inline bool operator==(const BlockType &other) const
		{
			return m_id == other.m_id;
		}

		static const BlockType &get(BlockTypeId id)
		{
			static const std::map<BlockTypeId, BlockType> blockTypes = {
				{BlockTypeId::NONE, BlockType(BlockTypeId::NONE, nullptr)},
				{BlockTypeId::AIR, BlockType(BlockTypeId::AIR, nullptr)},
				{BlockTypeId::STONE, BlockType(BlockTypeId::STONE, [](Block block, physics::Transform transform) { return singleTextureCube(block, transform, g::AtlasTexture::STONE); })},
				{BlockTypeId::DIRT, BlockType(BlockTypeId::DIRT, [](Block block, physics::Transform transform) { return singleTextureCube(block, transform, g::AtlasTexture::DIRT); })},
				{BlockTypeId::GRASS, BlockType(BlockTypeId::GRASS, [](Block block, physics::Transform transform) { return multiTextureCube(block, transform, g::AtlasTexture::GRASS_SIDE, g::AtlasTexture::GRASS_SIDE, g::AtlasTexture::GRASS_SIDE, g::AtlasTexture::GRASS_SIDE, g::AtlasTexture::GRASS, g::AtlasTexture::DIRT); })},
				{BlockTypeId::BEDROCK, BlockType(BlockTypeId::BEDROCK, [](Block block, physics::Transform transform) { return singleTextureCube(block, transform, g::AtlasTexture::BEDROCK); })}};
			return blockTypes.at(id);
		}
	};

	inline bool isSolid(BlockTypeId blockType) { return blockType != BlockTypeId::NONE && blockType != BlockTypeId::AIR; }
}