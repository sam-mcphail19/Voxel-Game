#include "block.hpp"

namespace voxel_game::world
{
	std::string toString(BlockTypeId id)
	{
		static const std::unordered_map<BlockTypeId, std::string> blockTypeStrings = {
			{BlockTypeId::NONE, "Null"},
			{BlockTypeId::AIR, "Air"},
			{BlockTypeId::BEDROCK, "Bedrock"},
			{BlockTypeId::STONE, "Stone"},
			{BlockTypeId::DIRT, "Dirt"},
			{BlockTypeId::GRASS, "Grass"},
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