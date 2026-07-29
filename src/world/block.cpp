#include "block.hpp"

namespace voxel_game::world
{
	std::string toString(BlockTypeId id)
	{
		static const std::unordered_map<BlockTypeId, std::string> blockTypeStrings = {
			{BlockTypeId::NONE, "Null"},
			{BlockTypeId::AIR, "Air"},
			{BlockTypeId::WATER, "Water"},
			{BlockTypeId::BEDROCK, "Bedrock"},
			{BlockTypeId::STONE, "Stone"},
			{BlockTypeId::DIRT, "Dirt"},
			{BlockTypeId::GRASS, "Grass"},
			{BlockTypeId::SAND, "Sand"},
			{BlockTypeId::GRAVEL, "Gravel"},
			{BlockTypeId::SANDSTONE, "Sandstone"},
			{BlockTypeId::MARSH_GRASS, "Marsh Grass"},
			{BlockTypeId::MUD, "Mud"},
			{BlockTypeId::RED_SANDSTONE, "Red Sandstone"},
			{BlockTypeId::SNOW, "Snow"},
			{BlockTypeId::ICE, "Ice"},
			{BlockTypeId::PODZOL, "Podzol"},
			{BlockTypeId::DRY_GRASS, "Dry Grass"},
			{BlockTypeId::RAINFOREST_GRASS, "Rainforest Grass"},
			{BlockTypeId::BASALT, "Basalt"},
			{BlockTypeId::LAVA, "Lava"},
			{BlockTypeId::SALT, "Salt"},
			{BlockTypeId::TERRACOTTA, "Terracotta"},
			{BlockTypeId::DECIDUOUS_LOG, "Deciduous Log"},
			{BlockTypeId::DECIDUOUS_LEAVES, "Deciduous Leaves"},
			{BlockTypeId::CONIFER_LOG, "Conifer Log"},
			{BlockTypeId::CONIFER_LEAVES, "Conifer Leaves"},
			{BlockTypeId::CACTUS, "Cactus"},
		};

		auto it = blockTypeStrings.find(id);
		if (it != blockTypeStrings.end())
		{
			return it->second;
		}
		else
		{
			return "Unknown";
		}
	}
}
