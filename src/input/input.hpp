#pragma once

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define MAX_KEYS 1024
#define MAX_BUTTONS 32

namespace voxel_game::input
{
	enum class KeyState
	{
		NONE,
		PRESSED,
		DOWN,
		RELEASED,
	};

	void update();
	void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
	void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
	void cursor_pos_callback(GLFWwindow *window, double mouseX, double mouseY);
	bool isKeyPressed(int keycode);
	bool isKeyReleased(int keycode);
	bool isButtonPressed(int button);
	void getMousePos(double &x, double &y);
}