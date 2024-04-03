#pragma once
#include "pch.h"
#include "Mesh.h"
#include "GraphicsEngine/GraphicsQueue.h"

class GraphicsDevice;
class GraphicsQueue;
class GraphicsSwapChain
{
	friend LRESULT __stdcall WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
public:
	static constexpr UINT FRAME_COUNT = 2;
	GraphicsSwapChain();
	GraphicsSwapChain(const GraphicsSwapChain&) = delete;
	~GraphicsSwapChain() = default;
	GraphicsSwapChain& operator=(const GraphicsSwapChain&) = delete;
	void Initialize(GraphicsDevice* aDevice, GraphicsQueue* aQueueToWaitFor);
	void Shutdown();
	UINT GetWidth() const;
	UINT GetHeight() const;
	float GetAspectRatio() const;
	bool IsResizing() const;
	bool IsRunning() const;
private:
	//GraphicsDevice* myDevice;
	HWND myHwnd;
	UINT myWidth;
	UINT myHeight;
	float myAspectRatio;
	bool myIsResizing;
	bool myIsRunning;
	std::thread myThread;
	GraphicsQueue myQueue;
	D3D12_RECT myScissorRect{};
	D3D12_VIEWPORT myViewport{};
	ComPtr<IDXGISwapChain3> mySwapChain;
	ComPtr<ID3D12CommandAllocator> myCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> myCommandList;
	ComPtr<ID3D12Resource> myBackBuffers[FRAME_COUNT];
	ComPtr<ID3D12Resource> myDepthStencilBuffer;
	ComPtr<ID3D12DescriptorHeap> myDsvHeap;
	ComPtr<ID3D12DescriptorHeap> myRtvHeap;
	UINT myFrameIndex;
	void Thread(GraphicsDevice* aDevice, GraphicsQueue* aQueueToWaitFor);
	void GuardedThread(GraphicsDevice* aDevice, GraphicsQueue* aQueueToWaitFor);
	LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);
};



inline UINT GraphicsSwapChain::GetWidth() const
{
	return myWidth;
}

inline UINT GraphicsSwapChain::GetHeight() const
{
	return myHeight;
}

inline float GraphicsSwapChain::GetAspectRatio() const
{
	return myAspectRatio;
}

inline bool GraphicsSwapChain::IsResizing() const
{
	return myIsResizing;
}

inline bool GraphicsSwapChain::IsRunning() const
{
	return myIsRunning;
}