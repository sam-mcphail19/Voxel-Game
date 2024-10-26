#pragma once

#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_impl_glfw.h"
#include "../vendor/imgui/imgui_impl_opengl3.h"
#include "window.hpp"

namespace voxel_game::graphics
{
	class UiRenderer
	{
	public:
		UiRenderer(const Window& window);

		void renderDebugInfo();
	};
}