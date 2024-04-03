#include "GraphicsQueue.h"
#include "GraphicsDevice.h"
#undef min
#undef max

GraphicsQueue::GraphicsQueue()
    : myFenceEventHandle(INVALID_HANDLE_VALUE)
    , myLastCompletedFenceValue(0)
    , myNextFenceValue(0)
{
}

GraphicsQueue::~GraphicsQueue()
{
    ::CloseHandle(myFenceEventHandle);
}

void GraphicsQueue::Initialize(GraphicsDevice* aDevice)
{
    myCommandQueue = aDevice->CreateDirectCommandQueue(L"CommandQueue::myCommandQueue");
    myFence = aDevice->CreateFence(L"CommandQueue::myFence");
    myLastCompletedFenceValue = 1;
    myNextFenceValue = 2;
    myFenceEventHandle = ::CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (myFenceEventHandle == nullptr)
    {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
    }
}

UINT64 GraphicsQueue::Signal()
{
    //todo: thread safety, but fuck it
    myCommandQueue->Signal(myFence.Get(), myNextFenceValue);
    return myNextFenceValue++;
}

UINT64 GraphicsQueue::ExecuteCommandList(const ComPtr<ID3D12CommandList>& aCommandList)
{	
    ID3D12CommandList* ppCommandList[1] = { aCommandList.Get() };
    myCommandQueue->ExecuteCommandLists(1, ppCommandList);
    return Signal();
}

UINT64 GraphicsQueue::PollCurrentFenceValue()
{
    myLastCompletedFenceValue = std::max(myLastCompletedFenceValue, myFence->GetCompletedValue());
    return myLastCompletedFenceValue;
}

bool GraphicsQueue::IsFenceComplete(const UINT64 aFenceValue)
{
    if (aFenceValue > myLastCompletedFenceValue)
    {
        PollCurrentFenceValue();
    }
    return aFenceValue <= myLastCompletedFenceValue;
}

void GraphicsQueue::InsertWait(const UINT64 aFenceValue)
{
    myCommandQueue->Wait(myFence.Get(), aFenceValue);
}

void GraphicsQueue::InsertWaitForQueueFence(GraphicsQueue& aOtherQueue, const UINT64 aFenceValue)
{
    myCommandQueue->Wait(aOtherQueue.myFence.Get(), aFenceValue);
}

void GraphicsQueue::InsertWaitForQueue(GraphicsQueue& aOtherQueue)
{
    myCommandQueue->Wait(aOtherQueue.myFence.Get(), aOtherQueue.GetNextFenceValue() - 1);
}

void GraphicsQueue::WaitForFence(UINT64 aFenceValue)
{
    if (IsFenceComplete(aFenceValue))
    {
        return;
    }

    //todo: thread safety, but fuck it
    ThrowIfFailed(myFence->SetEventOnCompletion(aFenceValue, myFenceEventHandle));
    ::WaitForSingleObjectEx(myFenceEventHandle, INFINITE, false);
    myLastCompletedFenceValue = aFenceValue;
}