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
		constexpr double BYTES_PER_MIB = 1024.0 * 1024.0;
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();
		{
			ImGui::Begin("Debug");
			ImGui::SetWindowPos("Debug", ImVec2(10, 10));
			ImGui::SetWindowSize("Debug", ImVec2(390, 390));
			ImGui::Text("Player position: (%d, %d, %d)", debugInfo.x, debugInfo.y, debugInfo.z);
			ImGui::Text("Surface height: %d (%+d from surface)",
				debugInfo.worldGen.surfaceHeight,
				debugInfo.y - debugInfo.worldGen.surfaceHeight);
			ImGui::Separator();
			ImGui::Text("Biome: %s", debugInfo.worldGen.biome.c_str());
			ImGui::Text("Terrain: %s", debugInfo.worldGen.terrain.c_str());
			ImGui::Text("Climate C/E/T/H: %.2f / %.2f / %.2f / %.2f",
				debugInfo.worldGen.continentalness,
				debugInfo.worldGen.erosion,
				debugInfo.worldGen.temperature,
				debugInfo.worldGen.humidity);
			ImGui::Text("Range/core/peak/pass: %.2f / %.2f / %.2f / %.2f",
				debugInfo.worldGen.mountainRange,
				debugInfo.worldGen.mountainCore,
				debugInfo.worldGen.mountainPeak,
				debugInfo.worldGen.mountainPass);
			ImGui::Text("Valley/canyon: %.2f / %.2f",
				debugInfo.worldGen.valley,
				debugInfo.worldGen.canyon);
			ImGui::Text("Gully/talus/deposit: %.2f / %.2f / %.2f",
				debugInfo.worldGen.erosionGully,
				debugInfo.worldGen.talus,
				debugInfo.worldGen.deposition);
			ImGui::Text("River mask/flow: %.2f / %.2f",
				debugInfo.worldGen.river,
				debugInfo.worldGen.riverFlow);
			ImGui::Separator();
			ImGui::Text("Loaded chunks: %zu", debugInfo.memory.loadedChunkCount);
			const size_t trackedBytes = debugInfo.memory.voxelStorageBytes
				+ debugInfo.memory.estimatedGpuMeshBytes
				+ debugInfo.memory.pendingCpuMeshBytes;
			ImGui::Text("Tracked memory: %.1f MiB", trackedBytes / BYTES_PER_MIB);
			ImGui::Text("Meshes: %zu resident, %zu pending",
				debugInfo.memory.residentMeshCount,
				debugInfo.memory.pendingMeshCount);
			ImGui::Text("Workers: %zu active, %zu queued",
				debugInfo.generation.activeWorkerCount,
				debugInfo.generation.queuedJobCount);
			ImGui::End();
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}
