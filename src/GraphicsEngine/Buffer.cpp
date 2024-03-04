#include "GraphicsEngine/Buffer.h"

Buffer::Buffer() = default;
Buffer::Buffer(Buffer&&) = default;
Buffer::~Buffer() = default;
Buffer& Buffer::operator=(Buffer&&) = default;

D3D12_GPU_VIRTUAL_ADDRESS Buffer::GetGPUVirtualAddress()
{
    return myBufferGPU->GetGPUVirtualAddress();
}

void Buffer::Upload(const void* aBlob, UINT aBlobByteSize)
{
    D3D12_RANGE range{ 0, 0 };
    void* pData = nullptr;

    myBufferGPU->Map(0, &range, &pData);
    std::memcpy(pData, aBlob, aBlobByteSize);
    myBufferGPU->Unmap(0, nullptr);
}