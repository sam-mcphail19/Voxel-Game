#pragma once

#include <string>
#include <vector>

namespace voxel_game::utils
{
	std::string read_file(std::string filepath);
	std::vector<std::string> walkPath(std::string path);
	std::string getFileName(std::string path);
}
