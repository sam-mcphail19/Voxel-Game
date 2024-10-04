#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include "../util/fileUtils.hpp"
#include "../util/log.hpp"
#include "../constants.hpp"

namespace voxel_game::graphics
{
	class Shader
	{
	private:
		GLuint m_id;

	private:
		GLuint load(std::string vertPath, std::string fragPath);
		bool compileShader(const char *source, GLuint &shader);

	public:
		Shader(std::string vertPath, std::string fragPath);
		~Shader();

		void bind();
		void unbind();

		GLint getUniformLocation(const GLchar *name);

		void setUniform1f(const GLchar *name, float value);
		void setUniform1i(const GLchar *name, int value);
		void setUniform2f(const GLchar *name, const glm::vec2 &vector);
		void setUniform3f(const GLchar *name, const glm::vec3 &vector);
		void setUniform3i(const GLchar *name, int x, int y, int z);
		void setUniform4f(const GLchar *name, const glm::vec4 &vector);
		void setUniformMat4(const GLchar *name, const glm::mat4 &matrix);
	};
}