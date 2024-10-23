#include "window.hpp"

namespace voxel_game::graphics
{
	// TODO: Change these to use the logger
	void errorCallback(int error, const char *description)
	{
		std::cerr << "GLFW error " << error << ": " << description << std::endl;
	}

	void debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
	{
		std::cerr << "OpenGL debug message: " << message << std::endl;
	}

	Window::Window(std::string name, int width, int height)
	{
		m_name = name;
		m_width = width;
		m_height = height;
		m_isWireframe = false;

		if (!init())
		{
			glfwTerminate();
		}

		glfwSetErrorCallback(errorCallback);
		//glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(debugCallback, nullptr);

		centerCursor();
		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	Window::~Window()
	{
		glfwTerminate();
	}

	void window_resize(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	bool Window::init()
	{
		if (!glfwInit())
		{
			std::cout << "Could not initialize GLFW" << std::endl;
			return false;
		}

		m_window = glfwCreateWindow(m_width, m_height, m_name.c_str(), nullptr, nullptr);

		if (!m_window)
		{
			std::cout << "Failed to create window" << std::endl;
			return false;
		}

		glfwMakeContextCurrent(m_window);

		if (glewInit() != GLEW_OK)
		{
			std::cout << "Could not initialize GLEW!" << std::endl;
			return false;
		}

		glfwSwapInterval(0);

		glfwSetWindowSizeCallback(m_window, window_resize);
		glfwSetKeyCallback(m_window, input::key_callback);
		glfwSetMouseButtonCallback(m_window, input::mouse_button_callback);
		glfwSetCursorPosCallback(m_window, input::cursor_pos_callback);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		return true;
	}

	void Window::update()
	{
		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			std::cout << "OpenGL Error: " << error << std::endl;
		}

		glfwSwapBuffers(m_window);
		glfwPollEvents();

		if (input::isKeyPressed(GLFW_KEY_ESCAPE))
		{
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		if (input::isKeyReleased(GLFW_KEY_Z))
		{
			toggleWireframe();
		}
	}

	bool Window::closed()
	{
		return glfwWindowShouldClose(m_window);
	}

	void Window::setBackground(float r, float g, float b)
	{
		glClearColor(r, g, b, 1.0f);
	}

	void Window::setTitle(std::string title)
	{
		glfwSetWindowTitle(m_window, title.c_str());
	}

	void Window::toggleWireframe()
	{
		m_isWireframe = !m_isWireframe;
		glPolygonMode(GL_FRONT_AND_BACK, m_isWireframe ? GL_LINE : GL_FILL);
	}

	void Window::centerCursor()
	{
		double x = m_width / 2;
		double y = m_height / 2;
		glfwSetCursorPos(m_window, x, y);
		input::cursor_pos_callback(m_window, x, y);
	}
}