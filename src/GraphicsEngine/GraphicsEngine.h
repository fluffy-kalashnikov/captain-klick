#pragma once
#include "pch.h"
#include "Math/Mat4.h"
#include "Math/Vec2.h"

class GraphicsEngine
{
public:
	static constexpr UINT FRAME_COUNT = 2;
	void OnInitialize();
	void OnUpdate(float aDeltaSeconds, float aTimeSeconds, const Mat4& aToViewMatrix, const Mat4& aToProjectionMatrix);
	void OnRender();
	void OnDestroy();
private:
	D3D12_RECT myScissorRect{};
	D3D12_VIEWPORT myViewport{};
	/** pipeline */
	ComPtr<ID3D12Debug1> myDebug;
	ComPtr<ID3D12Device> myDevice;
	ComPtr<IDXGISwapChain3> mySwapChain;
	ComPtr<IDXGIFactory> myFactory;
	ComPtr<IDXGIAdapter> myAdapter;
	ComPtr<ID3D12CommandAllocator> myCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> myCommandList;
	ComPtr<ID3D12CommandQueue> myCommandQueue;
	ComPtr<ID3D12RootSignature> myRootSignature;
	ComPtr<ID3D12Resource> myBackBuffers[FRAME_COUNT];
	ComPtr<ID3D12Resource> myDepthStencilBuffer;
	ComPtr<ID3D12PipelineState> myPipelineState;
	/** resources */
	ComPtr<ID3D12DescriptorHeap> myRtvHeap;
	ComPtr<ID3D12DescriptorHeap> myDsvHeap;
	ComPtr<ID3D12DescriptorHeap> myCbvSrvHeap;
	SIZE_T myRtvDescriptorSize{};
	SIZE_T myDsvDescriptorSize{};
	SIZE_T myCbvSrvDescriptorSize{};
	ComPtr<ID3D12Resource> myInstanceConstantBuffer;
	ComPtr<ID3D12Resource> myPassConstantBuffer;
	ComPtr<ID3D12Resource> myVertexBuffer;
	ComPtr<ID3D12Resource> myIndexBuffer;
	D3D12_VERTEX_BUFFER_VIEW myVertexBufferView{};
	D3D12_INDEX_BUFFER_VIEW myIndexBufferView{};
	/** synchronization */
	UINT myFrameIndex{};
	HANDLE myFenceEvent{};
	ComPtr<ID3D12Fence> myFence;
	UINT64 myFenceValue{};
	void LoadPipeline();
	void LoadAssets();
	void PopulateCommandList();
	void WaitForPreviousFrame();
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;
};