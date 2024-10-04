#include "shader.hpp"

namespace voxel_game::graphics
{
	std::string shaderPath("res/shader/");

	Shader::Shader(std::string vertPath, std::string fragPath)
	{
		m_id = load(vertPath, fragPath);
	}

	Shader::~Shader()
	{
		glDeleteProgram(m_id);
	}

	GLuint Shader::load(std::string vertPath, std::string fragPath)
	{
		GLuint id = glCreateProgram();
		GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
		GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

		if (!compileShader(voxel_game::utils::read_file(shaderPath + vertPath).c_str(), vertShader) ||
			!compileShader(voxel_game::utils::read_file(shaderPath + fragPath).c_str(), fragShader))
		{
			return 0;
		}

		glAttachShader(id, vertShader);
		glAttachShader(id, fragShader);

		glLinkProgram(id);
		glValidateProgram(id);

		glDeleteShader(vertShader);
		glDeleteShader(fragShader);

		return id;
	}

	bool Shader::compileShader(const char *source, GLuint &shader)
	{
		glShaderSource(shader, 1, &source, NULL);
		glCompileShader(shader);

		GLint compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

		if (!compiled)
		{
			GLint length;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
			std::vector<char> error(length);
			glGetShaderInfoLog(shader, length, &length, &error[0]);
			std::cout << &error[0] << std::endl;
			glDeleteShader(shader);
			return false;
		}

		return true;
	}

	void Shader::bind()
	{
		glUseProgram(m_id);
	}

	void Shader::unbind()
	{
		glUseProgram(0);
	}

	GLint Shader::getUniformLocation(const GLchar *name)
	{
		return glGetUniformLocation(m_id, name);
	}

	void Shader::setUniform1f(const GLchar *name, float value)
	{
		glUniform1f(getUniformLocation(name), value);
	}

	void Shader::setUniform1i(const GLchar *name, int value)
	{
		glUniform1i(getUniformLocation(name), value);
	}

	void Shader::setUniform2f(const GLchar *name, const glm::vec2 &vector)
	{
		glUniform2f(getUniformLocation(name), vector.x, vector.y);
	}

	void Shader::setUniform3f(const GLchar *name, const glm::vec3 &vector)
	{
		glUniform3f(getUniformLocation(name), vector.x, vector.y, vector.z);
	}

	void Shader::setUniform3i(const GLchar *name, int x, int y, int z)
	{
		glUniform3i(getUniformLocation(name), x, y, z);
	}

	void Shader::setUniform4f(const GLchar *name, const glm::vec4 &vector)
	{
		glUniform4f(getUniformLocation(name), vector.x, vector.y, vector.z, vector.w);
	}

	void Shader::setUniformMat4(const GLchar *name, const glm::mat4 &matrix)
	{
		glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
	}
}