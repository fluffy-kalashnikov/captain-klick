#pragma once
#include "pch.h"
#include "GraphicsEngine/Buffer.h"

class GraphicsDevice
{
public:
	void Initialize(const ComPtr<ID3D12Device>& aDevice);
	//Buffer CreateDefaultBuffer();
	template<class T>
	Buffer CreateUploadBuffer(std::wstring aName);
private:
	ComPtr<ID3D12Device> myDevice;
	Buffer CreateUploadBuffer(std::wstring aName, UINT aWidth);
};

template<class T>
Buffer GraphicsDevice::CreateUploadBuffer(std::wstring aName)
{
	return CreateUploadBuffer(aName, sizeof(T));
}