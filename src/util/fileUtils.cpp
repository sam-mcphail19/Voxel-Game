#include "fileUtils.hpp"

#include <dirent.h>
#include <sys/stat.h>

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

	std::vector<std::string> walkPath(std::string path)
	{
		std::vector<std::string> paths;
		DIR* dir = opendir(path.c_str());
		if (dir == nullptr)
		{
			perror("opendir");
			return paths;
		}

		dirent* entry;
		while ((entry = readdir(dir)) != nullptr)
		{
			std::string name = entry->d_name;
			if (name == "." || name == "..")
			{
				continue;
			}

			std::string fullPath = path + "/" + name;
			struct stat statBuf;
			if (stat(fullPath.c_str(), &statBuf) == 0 && S_ISREG(statBuf.st_mode))
			{
				paths.push_back(fullPath);
			}
		}

		closedir(dir);
		return paths;
	}

	std::string getFileName(std::string path)
	{
		size_t slashPos = path.find_last_of("/\\");
		size_t nameStart = slashPos == std::string::npos ? 0 : slashPos + 1;
		size_t dotPos = path.find_last_of('.');
		if (dotPos == std::string::npos || dotPos < nameStart)
		{
			dotPos = path.size();
		}

		return path.substr(nameStart, dotPos - nameStart);
	}
}
