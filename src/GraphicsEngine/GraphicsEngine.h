#pragma once
#include "pch.h"
#include "Camera.h"
#include "Math/Mat4.h"
#include "Math/Vec2.h"
#include "GraphicsEngine/Mesh.h"
#include "GraphicsEngine/GraphicsDevice.h"
#include "GraphicsEngine/GraphicsQueue.h"

class GraphicsEngine
{
public:
	static constexpr UINT FRAME_COUNT = 2;
	void Initialize(GraphicsDevice* aDevice);
	void Update(const GraphicsSwapChain* aSwapChain, Camera aCamera);
	void Resize(UINT aWidth, UINT aHeight);
	void Shutdown();
	GraphicsQueue& GetGraphicsQueue();
private:
	GraphicsDevice* myDevice;
	GraphicsQueue myQueue;
	D3D12_RECT myScissorRect{};
	D3D12_VIEWPORT myViewport{};
	UINT myAllocatorIndex{};
	ComPtr<ID3D12CommandAllocator> myCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> myCommandList;
	ComPtr<ID3D12Resource> mySceneBuffer;
	ComPtr<ID3D12Resource> myDepthStencilBuffer;
	ComPtr<ID3D12DescriptorHeap> myRtvHeap;
	ComPtr<ID3D12DescriptorHeap> myDsvHeap;
	ComPtr<ID3D12DescriptorHeap> myCbvSrvHeap;

	std::vector<Buffer> myInstanceConstantBuffers;
	Buffer myPassConstantBuffer;
};