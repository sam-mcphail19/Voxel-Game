#pragma once

#include <iostream>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../input/input.hpp"

namespace voxel_game::graphics
{
	class Window
	{
	private:
		std::string  m_name;
		int m_width, m_height;
		GLFWwindow *m_window;
		bool m_isWireframe;

	public:
		Window(std::string name, int width, int height);
		~Window();
		void update();
		bool closed();
		void setBackground(float r, float g, float b);
		void setTitle(std::string);
		void toggleWireframe();
		inline GLFWwindow *getWindow() const { return m_window; };
		inline int getWidth() const { return m_width; };
		inline int getHeight() const { return m_height; };

	private:
		void centerCursor(); 
		bool init();
	};
}