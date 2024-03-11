#pragma once
#include "pch.h"
#include "GraphicsEngine/Buffer.h"

class GraphicsDevice
{
public:
	static constexpr UINT FRAME_COUNT = 2;
	GraphicsDevice() = default;
	GraphicsDevice(const GraphicsDevice&) = delete;
	~GraphicsDevice() = default;
	GraphicsDevice& operator=(const GraphicsDevice&) = delete;
	void Initialize();
	void Shutdown();
	[[nodiscard]] ComPtr<ID3D12GraphicsCommandList> BeginFrame();
	void EndFrame(ComPtr<ID3D12GraphicsCommandList>&& aCommandList);
	template<class T>
	Buffer CreateDefaultBuffer(const std::wstring& aName, const T& aBuffer);
	template<class T, std::size_t N>
	Buffer CreateDefaultBuffer(const std::wstring& aName, const T(&aBuffer)[N]);
	template<class T>
	Buffer CreateUploadBuffer(const std::wstring& aName);
	void WaitForGPU();
private:
	D3D12_RECT myScissorRect{};
	D3D12_VIEWPORT myViewport{};
	/** pipeline */
	ComPtr<ID3D12Debug1> myDebug;
	ComPtr<ID3D12InfoQueue> myInfoQueue;
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
	ComPtr<ID3D12PipelineState> myCubePipelineState;
	/** resources */
	ComPtr<ID3D12DescriptorHeap> myRtvHeap;
	ComPtr<ID3D12DescriptorHeap> myDsvHeap;
	ComPtr<ID3D12DescriptorHeap> myCbvSrvHeap;
	UINT myRtvDescriptorSize{};
	UINT myDsvDescriptorSize{};
	UINT myCbvSrvDescriptorSize{};
	/** synchronization */
	UINT myFrameIndex{};
	HANDLE myFenceEvent{};
	ComPtr<ID3D12Fence> myFence;
	UINT64 myFenceValue{};
	void InitDevice();
	void InitPipeline();
	Buffer CreateDefaultBuffer(const std::wstring& aName, const void* aBuffer, UINT aBufferByteSize, UINT aBufferByteStride);
	Buffer CreateUploadBuffer(const std::wstring& aName, UINT aBufferByteSize, UINT aBufferByteStride);
};

template<class T>
Buffer GraphicsDevice::CreateDefaultBuffer(const std::wstring& aName, const T& aBuffer)
{
	static_assert(std::is_trivially_copyable_v<T>);
	return CreateDefaultBuffer(aName, &aBuffer, sizeof(aBuffer), sizeof(aBuffer));
}

template<class T, std::size_t N>
Buffer GraphicsDevice::CreateDefaultBuffer(const std::wstring& aName, const T(&aBuffer)[N])
{
	static_assert(std::is_trivially_copyable_v<T>);
	return CreateDefaultBuffer(aName, aBuffer, sizeof(aBuffer), sizeof(aBuffer[0]));
}

template<class T>
Buffer GraphicsDevice::CreateUploadBuffer(const std::wstring& aName)
{
	static_assert(std::is_trivially_copyable_v<T>);
	return CreateUploadBuffer(aName, sizeof(T), sizeof(T));
}