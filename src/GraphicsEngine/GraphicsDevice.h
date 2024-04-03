#pragma once
#include "pch.h"
#include "GraphicsEngine/Buffer.h"
#include "GraphicsEngine/GraphicsSwapChain.h"
#include "GraphicsEngine/GraphicsQueue.h"

class GraphicsDevice
{
public:
	GraphicsDevice() = default;
	GraphicsDevice(const GraphicsDevice&) = delete;
	~GraphicsDevice() = default;
	GraphicsDevice& operator=(const GraphicsDevice&) = delete;
	void Initialize();
	ComPtr<ID3D12Resource> CreateTexture2D(const std::wstring& aName, UINT aWidth, UINT aHeight, 
		DXGI_FORMAT aFormat = DXGI_FORMAT_R8G8B8A8_UNORM, 
		D3D12_RESOURCE_STATES aResourceStates = D3D12_RESOURCE_STATE_RENDER_TARGET, 
		D3D12_RESOURCE_FLAGS aResourceFlags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
	template<class T>
	Buffer CreateDefaultBuffer(const std::wstring& aName, const ComPtr<ID3D12GraphicsCommandList>& aCommandList, const T& aBuffer);
	template<class T, std::size_t N>
	Buffer CreateDefaultBuffer(const std::wstring& aName, const ComPtr<ID3D12GraphicsCommandList>& aCommandList, const T(&aBuffer)[N]);
	template<class T>
	Buffer CreateUploadBuffer(const std::wstring& aName);
	ComPtr<ID3D12RootSignature> CreateRootSignature(const std::wstring& aName,
		const D3D12_ROOT_SIGNATURE_DESC& aRootSignatureDesc);
	ComPtr<ID3D12PipelineState> CreatePipelineState(const std::wstring& aName,
		const D3D12_GRAPHICS_PIPELINE_STATE_DESC& aPipelineStateDesc);
	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(const std::wstring& aName, const D3D12_DESCRIPTOR_HEAP_DESC& aDescriptorHeapDesc);
	ComPtr<ID3D12Fence> CreateFence(const std::wstring& aName);
	ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(const std::wstring& aName);
	ComPtr<ID3D12GraphicsCommandList> CreateGraphicsCommandList(const std::wstring& aName, const ComPtr<ID3D12PipelineState>& aPipelineState, const ComPtr<ID3D12CommandAllocator>& aCommandAllocator);
	ComPtr<ID3D12CommandQueue> CreateDirectCommandQueue(const std::wstring& aName);
	void CreateRenderTargetView(const ComPtr<ID3D12Resource>& aResource, D3D12_CPU_DESCRIPTOR_HANDLE aRtvHeapHandle);
	void CreateDepthStencilView(const ComPtr<ID3D12Resource>& aResource, D3D12_CPU_DESCRIPTOR_HANDLE aDsvHeapHandle);
	ComPtr<IDXGISwapChain3> CreateSwapChain(const ComPtr<ID3D12CommandQueue>& aCommandQueue, HWND aHwnd, UINT aBufferCount);
	UINT RtvDescriptorSize() const;
	UINT DsvDescriptorSize() const;
	UINT CbvSrvDescriptorSize() const;
private:
	//D3D12_RECT myScissorRect{};
	//D3D12_VIEWPORT myViewport{};
	/** pipeline */
	ComPtr<ID3D12Debug> myDebug;
	ComPtr<ID3D12InfoQueue> myInfoQueue;
	ComPtr<ID3D12Device> myDevice;
	ComPtr<IDXGIFactory3> myFactory;
	ComPtr<IDXGIAdapter> myAdapter;
	//ComPtr<ID3D12CommandAllocator> myCommandAllocator;
	//ComPtr<ID3D12GraphicsCommandList> myCommandList;
	//ComPtr<ID3D12CommandQueue> myCommandQueue;
	//ComPtr<ID3D12Resource> mySceneBuffer; //uninitialized;
	//ComPtr<ID3D12Resource> myDepthStencilBuffer;
	/** resources */
	//ComPtr<ID3D12DescriptorHeap> myRtvHeap;
	//ComPtr<ID3D12DescriptorHeap> myDsvHeap;
	//ComPtr<ID3D12DescriptorHeap> myCbvSrvHeap;
	UINT myRtvDescriptorSize{};
	UINT myDsvDescriptorSize{};
	UINT myCbvSrvDescriptorSize{};
	/** synchronization */
	//HANDLE myFenceEvent{};
	//ComPtr<ID3D12Fence> myFence;
	//UINT64 myFenceValue{};
	Buffer CreateDefaultBuffer(const std::wstring& aName, const ComPtr<ID3D12GraphicsCommandList>& aCommandList, const void* aBuffer, UINT aBufferByteSize, UINT aBufferByteStride);
	Buffer CreateUploadBuffer(const std::wstring& aName, UINT aBufferByteSize, UINT aBufferByteStride);
};

template<class T>
Buffer GraphicsDevice::CreateDefaultBuffer(const std::wstring& aName, 
	const ComPtr<ID3D12GraphicsCommandList>& aCommandList, const T& aBuffer)
{
	static_assert(std::is_trivially_copyable_v<T>);
	return CreateDefaultBuffer(aName, aCommandList, &aBuffer, sizeof(aBuffer), sizeof(aBuffer));
}

template<class T, std::size_t N>
Buffer GraphicsDevice::CreateDefaultBuffer(const std::wstring& aName, 
	const ComPtr<ID3D12GraphicsCommandList>& aCommandList, const T(&aBuffer)[N])
{
	static_assert(std::is_trivially_copyable_v<T>);
	return CreateDefaultBuffer(aName, aCommandList, aBuffer, sizeof(aBuffer), sizeof(aBuffer[0]));
}

template<class T>
Buffer GraphicsDevice::CreateUploadBuffer(const std::wstring& aName)
{
	static_assert(std::is_trivially_copyable_v<T>);
	return CreateUploadBuffer(aName, sizeof(T), sizeof(T));
}

inline UINT GraphicsDevice::RtvDescriptorSize() const
{
	return myRtvDescriptorSize;
}

inline UINT GraphicsDevice::DsvDescriptorSize() const
{
	return myDsvDescriptorSize;
}

inline UINT GraphicsDevice::CbvSrvDescriptorSize() const
{
	return myCbvSrvDescriptorSize;
}