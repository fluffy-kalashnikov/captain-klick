#include "GraphicsDevice.h"


void GraphicsDevice::Initialize(const ComPtr<ID3D12Device>& aDevice)
{
	myDevice = aDevice;
}

//Buffer GraphicsDevice::CreateDefaultBuffer()
//{
//
//}

Buffer GraphicsDevice::CreateUploadBuffer(std::wstring aName, const UINT aWidth)
{
    D3D12_HEAP_PROPERTIES uploadHeap{};
    uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC bufferDesc{};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    bufferDesc.Width = aWidth;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.SampleDesc.Quality = 0;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ComPtr<ID3D12Resource> bufferGPU;
    ThrowIfFailed(myDevice->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE,
        &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&bufferGPU)));
    ThrowIfFailed(bufferGPU->SetName(aName.c_str()));

    Buffer buffer;
    buffer.myBufferCPU = nullptr;
    buffer.myBufferGPU = std::move(bufferGPU);
    buffer.myBufferUpload = nullptr;
    return buffer;
}