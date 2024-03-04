#pragma once
#include "pch.h"

class Buffer
{
	friend class GraphicsDevice;
public:
	Buffer();
	Buffer(const Buffer&) = delete;
	Buffer(Buffer&&);
	~Buffer();
	template<class T>
	void Upload(const T& aStruct);
	template<class T, std::size_t N>
	void Upload(const T(& aStruct)[N]);
	Buffer& operator=(const Buffer&) = delete;
	Buffer& operator=(Buffer&&);
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress();
private:
	ComPtr<ID3DBlob> myBufferCPU;
	ComPtr<ID3D12Resource> myBufferGPU;
	ComPtr<ID3D12Resource> myBufferUpload;
	void Upload(const void* aBlob, UINT aBlobByteSize);
};

template<class T>
void Buffer::Upload(const T& aStruct)
{
	static_assert(std::is_trivially_copyable_v<T>);
	Upload(&aStruct, sizeof(aStruct));
}

template<class T, std::size_t N>
void Buffer::Upload(const T(&aStruct)[N])
{
	static_assert(std::is_trivially_copyable_v<T>);
	Upload(aStruct, sizeof(aStruct));
}