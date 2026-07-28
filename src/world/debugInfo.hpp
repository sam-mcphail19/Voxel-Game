#pragma once
#include <cstddef>
#include <array>
#include <cstdint>
#include <string>

namespace voxel_game::world
{
	struct MemoryDiagnostics
	{
		size_t loadedChunkCount = 0;
		size_t voxelStorageBytes = 0;
		size_t residentMeshCount = 0;
		size_t pendingMeshCount = 0;
		size_t meshVertexCount = 0;
		size_t meshIndexCount = 0;
		size_t estimatedGpuMeshBytes = 0;
		size_t pendingCpuMeshBytes = 0;
	};

	struct StageTiming
	{
		uint64_t totalNanoseconds = 0;
		uint64_t callCount = 0;
		uint64_t maximumNanoseconds = 0;
		uint64_t p95Nanoseconds = 0;
	};

	enum class ProfileStage : size_t
	{
		ChunkGeneration,
		ColumnSampling,
		SurfaceHeightSearch,
		VoxelGeneration,
		HydrologyRegion,
		RepresentativeGridBuild,
		OpaqueMaskBuild,
		GreedyMergeAndEmit,
		WaterMeshing,
		MeshUpload,
		Count
	};

	enum class ProfileCounter : size_t
	{
		TerrainDensityEvaluations,
		TerrainDensityFarRejections,
		Terrain3dNoiseSamples,
		TerrainAirCaveSkips,
		CaveEvaluations,
		CaveColumnSamples,
		Cave3dNoiseSamples,
		Ravine2dNoiseSamples,
		TunnelBoundarySkips,
		BlockAssignments,
		RepresentativeBlockSamples,
		FaceVisibilityChecks,
		LocalMeshingBlockReads,
		NeighbourChunkReads,
		WorldFallbackReads,
		CandidateFaces,
		EmittedGreedyQuads,
		FullLodBuilds,
		HalfLodBuilds,
		QuarterLodBuilds,
		HydrologyThreadLocalHits,
		HydrologyCacheHits,
		HydrologyCacheWaits,
		HydrologyRegionsBuilt,
		Count
	};

	struct GenerationDiagnostics
	{
		static constexpr size_t STAGE_COUNT = static_cast<size_t>(ProfileStage::Count);
		std::array<StageTiming, STAGE_COUNT> stages{};
		static constexpr size_t COUNTER_COUNT = static_cast<size_t>(ProfileCounter::Count);
		std::array<uint64_t, COUNTER_COUNT> counters{};
		size_t activeWorkerCount = 0;
		size_t queuedJobCount = 0;
	};

	struct WorldGenDiagnostics
	{
		std::string biome = "Unknown";
		std::string terrain = "Unknown";
		int surfaceHeight = 0;
		float continentalness = 0.0f;
		float erosion = 0.0f;
		float temperature = 0.0f;
		float humidity = 0.0f;
		float mountainRange = 0.0f;
		float mountainCore = 0.0f;
		float mountainPeak = 0.0f;
		float mountainPass = 0.0f;
		float valley = 0.0f;
		float canyon = 0.0f;
		float erosionGully = 0.0f;
		float talus = 0.0f;
		float deposition = 0.0f;
		float river = 0.0f;
		float riverFlow = 0.0f;
	};

	struct DebugInfo
	{
		int x = 0, y = 0, z = 0;
		MemoryDiagnostics memory;
		GenerationDiagnostics generation;
		WorldGenDiagnostics worldGen;
	};
}
