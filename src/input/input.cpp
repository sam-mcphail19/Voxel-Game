#include "input.hpp"

namespace voxel_game::input
{
	KeyState keys[MAX_KEYS];
	KeyState buttons[MAX_BUTTONS];
	double mouseX, mouseY;

	void refreshKeyStates()
	{
		for (int i = 0; i < MAX_KEYS; i++)
		{
			if (keys[i] == KeyState::RELEASED)
			{
				keys[i] = KeyState::NONE;
			}
		}
	}

	void refreshButtonStates()
	{
		for (int i = 0; i < MAX_BUTTONS; i++)
		{
			if (buttons[i] == KeyState::RELEASED)
			{
				buttons[i] = KeyState::NONE;
			}
		}
	}

	void update()
	{
		refreshButtonStates();
		refreshKeyStates();
	}

	void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		switch (action)
		{
		case GLFW_RELEASE:
			keys[key] = KeyState::RELEASED;
			break;
		case GLFW_PRESS:
			keys[key] = KeyState::PRESSED;
			break;
		case GLFW_REPEAT:
			keys[key] = KeyState::DOWN;
			break;
		default:
			break;
		}
	}

	void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
	{
		switch (action)
		{
		case GLFW_RELEASE:
			buttons[button] = KeyState::RELEASED;
			break;
		case GLFW_PRESS:
			buttons[button] = KeyState::PRESSED;
			break;
		case GLFW_REPEAT:
			buttons[button] = KeyState::DOWN;
			break;
		default:
			break;
		}
	}

	void cursor_pos_callback(GLFWwindow *window, double xPos, double yPos)
	{
		mouseX = xPos;
		mouseY = yPos;
	}

	bool isKeyPressed(int keycode)
	{
		return keys[keycode] == KeyState::PRESSED || keys[keycode] == KeyState::DOWN;
	}

	bool isKeyReleased(int keycode)
	{
		return keys[keycode] == KeyState::RELEASED;
	}

	bool isButtonPressed(int button)
	{
		return buttons[button] == KeyState::PRESSED || buttons[button] == KeyState::DOWN;
	}

	void getMousePos(double &x, double &y)
	{
		x = mouseX;
		y = mouseY;
	}
}