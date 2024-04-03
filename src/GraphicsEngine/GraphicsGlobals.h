#pragma once
#include "pch.h"
#include "Mesh.h"

class GraphicsDevice;
class GraphicsQueue;
namespace GraphicsGlobals
{
	inline ComPtr<ID3D12RootSignature> globalRootSignature;
	inline ComPtr<ID3D12PipelineState> globalModelPipeline;
	inline ComPtr<ID3D12PipelineState> globalWorkPipeline;
	inline Mesh globalCubeMesh;
	inline Mesh globalPlaneMesh;
	void Create(GraphicsDevice* aDevice, const ComPtr<ID3D12GraphicsCommandList>& aCommandList);
	void Destroy();
};

