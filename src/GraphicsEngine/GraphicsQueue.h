#pragma once
#include "pch.h"

class GraphicsDevice;
class GraphicsQueue
{
public:
	GraphicsQueue();
	GraphicsQueue(const GraphicsQueue&) = delete;
	~GraphicsQueue();
	GraphicsQueue& operator=(const GraphicsQueue&) = delete;
	void Initialize(GraphicsDevice* aDevice);
	UINT64 Signal();
	UINT64 ExecuteCommandList(const ComPtr<ID3D12CommandList>& aCommandList);
	UINT64 PollCurrentFenceValue();
	bool IsFenceComplete(UINT64 aFenceValue);
	void InsertWait(UINT64 aFenceValue);
	void InsertWaitForQueueFence(GraphicsQueue& aOtherQueue, UINT64 aFenceValue);
	void InsertWaitForQueue(GraphicsQueue& aOtherQueue);
	void WaitForFence(UINT64 aFenceValue);
	void WaitForIdle();
	UINT64 GetNextFenceValue() const;
	UINT64 GetPrevFenceValue() const;
	const ComPtr<ID3D12CommandQueue>& GetID3D12CommandQueue() const;
private:
	ComPtr<ID3D12CommandQueue> myCommandQueue;
	ComPtr<ID3D12Fence> myFence;
	HANDLE myFenceEventHandle;
	UINT64 myNextFenceValue;
	UINT64 myLastCompletedFenceValue;
};



inline UINT64 GraphicsQueue::GetNextFenceValue() const
{
	return myNextFenceValue;
}

inline UINT64 GraphicsQueue::GetPrevFenceValue() const
{
	return myLastCompletedFenceValue;
}

inline void GraphicsQueue::WaitForIdle()
{
	WaitForFence(myNextFenceValue - 1);
}

inline const ComPtr<ID3D12CommandQueue>& GraphicsQueue::GetID3D12CommandQueue() const
{
	return myCommandQueue;
}
