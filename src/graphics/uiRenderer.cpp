#include "uiRenderer.hpp"
#include <algorithm>

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
			ImGui::SetWindowSize("Debug", ImVec2(610, 650));
			ImGui::Text("Player position: (%d, %d, %d)", debugInfo.x, debugInfo.y, debugInfo.z);
			ImGui::Separator();
			ImGui::Text("Loaded chunks: %zu", debugInfo.memory.loadedChunkCount);
			ImGui::Text("Packed voxels: %.1f MiB",
				debugInfo.memory.voxelStorageBytes / BYTES_PER_MIB);
			ImGui::Text("Resident meshes: %zu", debugInfo.memory.residentMeshCount);
			ImGui::Text("Mesh vertices / indices: %zu / %zu",
				debugInfo.memory.meshVertexCount, debugInfo.memory.meshIndexCount);
			ImGui::Text("Estimated GPU meshes: %.1f MiB",
				debugInfo.memory.estimatedGpuMeshBytes / BYTES_PER_MIB);
			ImGui::Text("Pending meshes: %zu", debugInfo.memory.pendingMeshCount);
			ImGui::Text("Pending CPU mesh data: %.1f MiB",
				debugInfo.memory.pendingCpuMeshBytes / BYTES_PER_MIB);
			const size_t trackedBytes = debugInfo.memory.voxelStorageBytes
				+ debugInfo.memory.estimatedGpuMeshBytes
				+ debugInfo.memory.pendingCpuMeshBytes;
			ImGui::Text("Tracked total: %.1f MiB", trackedBytes / BYTES_PER_MIB);
			ImGui::Separator();
			ImGui::Text("World work during last ~1 second");
			auto showTiming = [&](const char* label, world::ProfileStage stage)
			{
				const size_t stageIndex = static_cast<size_t>(stage);
				const world::StageTiming& timing = debugInfo.generation.stages[stageIndex];
				const double totalMs = timing.totalNanoseconds / 1'000'000.0;
				const double averageMs = timing.callCount > 0
					? totalMs / static_cast<double>(timing.callCount)
					: 0.0;
				ImGui::Text("%s: %.1f total | %llu calls | %.2f avg | %.2f p95 | %.2f max ms",
					label,
					totalMs,
					static_cast<unsigned long long>(timing.callCount),
					averageMs,
					timing.p95Nanoseconds / 1'000'000.0,
					timing.maximumNanoseconds / 1'000'000.0);
			};
			showTiming("Whole chunk", world::ProfileStage::ChunkGeneration);
			showTiming("Column samples", world::ProfileStage::ColumnSampling);
			showTiming("Surface searches", world::ProfileStage::SurfaceHeightSearch);
			showTiming("Voxel density/fill", world::ProfileStage::VoxelGeneration);
			showTiming("Hydrology regions", world::ProfileStage::HydrologyRegion);
			showTiming("Representative grid", world::ProfileStage::RepresentativeGridBuild);
			showTiming("Opaque mask build", world::ProfileStage::OpaqueMaskBuild);
			showTiming("Greedy merge/emit", world::ProfileStage::GreedyMergeAndEmit);
			showTiming("Water meshing", world::ProfileStage::WaterMeshing);
			showTiming("Main-thread upload", world::ProfileStage::MeshUpload);

			auto counter = [&](world::ProfileCounter value)
			{
				return debugInfo.generation.counters[static_cast<size_t>(value)];
			};
			ImGui::Separator();
			ImGui::Text("Workers: %zu active, %zu queued",
				debugInfo.generation.activeWorkerCount,
				debugInfo.generation.queuedJobCount);
			const uint64_t densityEvaluations = counter(world::ProfileCounter::TerrainDensityEvaluations);
			const uint64_t farRejections = counter(world::ProfileCounter::TerrainDensityFarRejections);
			const uint64_t caveSkips = counter(world::ProfileCounter::TerrainAirCaveSkips);
			ImGui::Text("Density: %llu eval | %llu far reject | %llu terrain 3D noise",
				static_cast<unsigned long long>(densityEvaluations),
				static_cast<unsigned long long>(farRejections),
				static_cast<unsigned long long>(counter(world::ProfileCounter::Terrain3dNoiseSamples)));
			ImGui::Text("Caves: %llu skip | %llu eval | %llu columns | %llu 3D / %llu ravine 2D noise",
				static_cast<unsigned long long>(caveSkips),
				static_cast<unsigned long long>(counter(world::ProfileCounter::CaveEvaluations)),
				static_cast<unsigned long long>(counter(world::ProfileCounter::CaveColumnSamples)),
				static_cast<unsigned long long>(counter(world::ProfileCounter::Cave3dNoiseSamples)),
				static_cast<unsigned long long>(counter(world::ProfileCounter::Ravine2dNoiseSamples)));
			ImGui::Text("Protected tunnel skips: %llu | block assignments: %llu",
				static_cast<unsigned long long>(counter(world::ProfileCounter::TunnelBoundarySkips)),
				static_cast<unsigned long long>(counter(world::ProfileCounter::BlockAssignments)));
			const uint64_t candidateFaces = counter(world::ProfileCounter::CandidateFaces);
			const uint64_t emittedQuads = counter(world::ProfileCounter::EmittedGreedyQuads);
			const double mergedPercent = candidateFaces > 0
				? 100.0 * (candidateFaces - std::min(candidateFaces, emittedQuads))
					/ static_cast<double>(candidateFaces)
				: 0.0;
			ImGui::Text("Meshing: %llu candidates -> %llu quads (%.1f%% merged)",
				static_cast<unsigned long long>(candidateFaces),
				static_cast<unsigned long long>(emittedQuads),
				mergedPercent);
			ImGui::Text("Mesh reads: %llu representative | %llu visibility | %llu local | %llu neighbour | %llu fallback",
				static_cast<unsigned long long>(counter(world::ProfileCounter::RepresentativeBlockSamples)),
				static_cast<unsigned long long>(counter(world::ProfileCounter::FaceVisibilityChecks)),
				static_cast<unsigned long long>(counter(world::ProfileCounter::LocalMeshingBlockReads)),
				static_cast<unsigned long long>(counter(world::ProfileCounter::NeighbourChunkReads)),
				static_cast<unsigned long long>(counter(world::ProfileCounter::WorldFallbackReads)));
			ImGui::Text("LOD builds: %llu full | %llu half | %llu quarter",
				static_cast<unsigned long long>(counter(world::ProfileCounter::FullLodBuilds)),
				static_cast<unsigned long long>(counter(world::ProfileCounter::HalfLodBuilds)),
				static_cast<unsigned long long>(counter(world::ProfileCounter::QuarterLodBuilds)));
			ImGui::Text("Hydrology: %llu thread hits | %llu shared hits | %llu waits | %llu builds",
				static_cast<unsigned long long>(counter(world::ProfileCounter::HydrologyThreadLocalHits)),
				static_cast<unsigned long long>(counter(world::ProfileCounter::HydrologyCacheHits)),
				static_cast<unsigned long long>(counter(world::ProfileCounter::HydrologyCacheWaits)),
				static_cast<unsigned long long>(counter(world::ProfileCounter::HydrologyRegionsBuilt)));
			ImGui::End();
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}
