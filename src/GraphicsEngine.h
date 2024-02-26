#pragma once
#include "pch.h"

class GraphicsEngine
{
public:
	static constexpr UINT FRAME_COUNT = 2;
	void OnInitialize(HWND aHwnd);
	void OnUpdate();
	void OnRender();
	void OnDestroy();
private:
	HWND myHwnd{};
	D3D12_VIEWPORT myViewport{};
	D3D12_RECT myScissorRect{};
	UINT myWidth{};
	UINT myHeight{};
	FLOAT myAspectRatio{};
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
	ComPtr<ID3D12Resource> myRenderTargets[FRAME_COUNT];
	ComPtr<ID3D12PipelineState> myPipelineState;
	/** resources */
	ComPtr<ID3D12DescriptorHeap> myRtvHeap;
	ComPtr<ID3D12DescriptorHeap> myDsvHeap;
	ComPtr<ID3D12DescriptorHeap> myCbvSrvHeap;
	SIZE_T myRtvDescriptorSize{};
	SIZE_T myDsvDescriptorSize{};
	SIZE_T myCbvSrvDescriptorSize{};
	ComPtr<ID3D12Resource> myFrameConstantBuffer;
	ComPtr<ID3D12Resource> myVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW myVertexBufferView{};
	/** synchronization */
	UINT myFrameIndex{};
	HANDLE myFenceEvent{};
	ComPtr<ID3D12Fence> myFence;
	UINT64 myFenceValue{};
	void LoadPipeline();
	void LoadAssets();
	void PopulateCommandList();
	void WaitForPreviousFrame();
};