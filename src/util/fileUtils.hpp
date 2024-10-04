#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace voxel_game::utils
{
	std::string read_file(std::string filepath);
	std::vector<std::filesystem::path> walkPath(std::string path);
	std::string getFileName(std::filesystem::path path);
}