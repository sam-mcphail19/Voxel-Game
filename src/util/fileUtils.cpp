#include "fileUtils.hpp"

namespace voxel_game::utils
{
	std::string read_file(std::string filepath)
	{
		FILE *file = fopen(filepath.c_str(), "rb");

		if (file == nullptr)
		{
			perror("fopen");
			return "";
		}

		fseek(file, 0, SEEK_END);
		long length = ftell(file);
		fseek(file, 0, SEEK_SET);

		std::vector<char> buffer(length);
		fread(buffer.data(), 1, length, file);
		fclose(file);

		return std::string(buffer.data(), buffer.size());
	}

	std::vector<std::filesystem::path> walkPath(std::string path)
	{
		std::vector<std::filesystem::path> paths;
		for (const auto& entry : std::filesystem::directory_iterator(path)) {
			if (entry.is_regular_file()) {
            	paths.push_back(entry.path());
			}
		}
		return paths;
	}

	std::string getFileName(std::filesystem::path path)
	{
		return path.stem().string();
	}
}