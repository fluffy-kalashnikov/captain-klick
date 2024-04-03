#include "GraphicsEngine/Buffer.h"

Buffer::Buffer()
    : myBlobByteSize(0)
    , myBlobByteStride(0)
{
}

Buffer::Buffer(Buffer&&) = default;

Buffer::~Buffer() = default;

Buffer& Buffer::operator=(Buffer&&) = default;

UINT Buffer::SizeInBytes() const
{
    return myBlobByteSize;
}

UINT Buffer::StrideInBytes() const
{
    return myBlobByteStride;
}

D3D12_GPU_VIRTUAL_ADDRESS Buffer::GetGPUVirtualAddress()
{
    return myBlobGPU->GetGPUVirtualAddress();
}

void Buffer::Upload(const void* aBlob, UINT aBlobByteSize, UINT aBlobByteStride)
{
    CD3DX12_RANGE readRange(0, 0);
    void* pData = nullptr;

    myBlobGPU->Map(0, &readRange, &pData);
    std::memcpy(pData, aBlob, aBlobByteSize);
    myBlobGPU->Unmap(0, nullptr);
    myBlobByteSize = aBlobByteSize;
    myBlobByteStride = aBlobByteStride;
}