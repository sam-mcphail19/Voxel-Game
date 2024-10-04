#pragma once

#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <glm/vec3.hpp>
#include "../physics/transform.hpp"

namespace voxel_game::world
{
	enum class BlockTypeId
	{
		NONE,
		AIR,
		BEDROCK,
		STONE,
		DIRT,
		GRASS
	};

	std::string toString(BlockTypeId id);

	struct BlockPos
	{
		int x, y, z;

		friend std::ostream &operator<<(std::ostream &os, const BlockPos &pos)
		{
			os << "BlockPos(" << pos.x << ", " << pos.y << ", " << pos.z << ")";
			return os;
		}

		inline bool operator<(const BlockPos &other) const
		{
			if (x != other.x)
				return x < other.x;
			if (y != other.y)
				return y < other.y;
			return z < other.z;
		}
	};

	inline BlockPos operator+(const BlockPos &pos1, const BlockPos &pos2)
	{
		return {pos1.x + pos2.x, pos1.y + pos2.y, pos1.z + pos2.z};
	}

	inline void operator+=(BlockPos &pos1, const BlockPos &pos2)
	{
		pos1 = pos1 + pos2;
	}

	inline bool operator==(const BlockPos &pos1, const BlockPos &pos2)
	{
		return pos1.x == pos2.x && pos1.y == pos2.y && pos1.z == pos2.z;
	}

	inline bool operator!=(const BlockPos &pos1, const BlockPos &pos2)
	{
		return !(pos1 == pos2);
	}

	inline std::string operator+(const std::string &str, const BlockPos &pos)
	{
		std::ostringstream stream;
		stream << str << pos;
		return stream.str();
	}

	inline glm::vec3 toVec(BlockPos blockPos)
	{
		return glm::vec3(blockPos.x, blockPos.y, blockPos.z);
	}

	inline BlockPos toBlockPos(glm::vec3 vec)
	{
		return {(int)vec.x, (int)vec.y, (int)vec.z};
	}

	struct Block
	{
		BlockPos pos;
		BlockTypeId type;
	};

	static const Block NULL_BLOCK{{}, BlockTypeId::NONE};

	inline bool operator==(const Block &block1, const Block &block2)
	{
		return block1.pos == block2.pos && block1.type == block2.type;
	}

	inline bool operator!=(const Block &block1, const Block &block2)
	{
		return !(block1 == block2);
	}

	inline std::string operator+(std::string str, Block block)
	{
		return str + "Block(pos=" + block.pos + ", type=" + toString(block.type) + ")";
	}
}