#include "uiRenderer.hpp"

namespace voxel_game::graphics
{
	UiRenderer::UiRenderer(const Window& window)
	{
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;

		int width, height;
		glfwGetFramebufferSize(window.getWindow(), &width, &height);
		io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));

		ImGui_ImplGlfw_InitForOpenGL(window.getWindow(), true);
		ImGui_ImplOpenGL3_Init("#version 450");

		io.Fonts->AddFontDefault();
		io.Fonts->Build();
	}

	void UiRenderer::renderDebugInfo(world::DebugInfo debugInfo)
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();
		{
			ImGui::Begin("Debug");
			ImGui::Text("Player position: (%d, %d, %d)", debugInfo.x, debugInfo.y, debugInfo.z);
			ImGui::Text("C: %f", debugInfo.m_c);
			ImGui::Text("E: %f", debugInfo.m_e);
			ImGui::Text("PV: %f", debugInfo.m_pv);
			ImGui::End();
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}