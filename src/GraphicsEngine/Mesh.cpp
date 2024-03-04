#include "pch.h"
#include "GraphicsEngine/Mesh.h"
#include "GraphicsEngine/Vertex.h"

void Mesh::Init(ID3D12Device* aDevice, std::string_view aName, UINT aVertexByteStride, const void* aVertexBuffer, 
	UINT aVertexBufferByteSize, const void* aIndexBuffer, UINT aIndexBufferByteSize)
{
	try
	{
		myName = aName;
		myVertexByteStride = aVertexByteStride;
		myVertexBufferByteSize = aVertexBufferByteSize;
		myIndexBufferByteSize = aIndexBufferByteSize;
		
		D3D12_RANGE range{ 0, 0 };
		void* pDataBegin{};

		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

		try
		{
			D3D12_RESOURCE_DESC vertexBufferDesc{};
			vertexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			vertexBufferDesc.Alignment = 0;
			vertexBufferDesc.Width = aVertexBufferByteSize;
			vertexBufferDesc.Height = 1;
			vertexBufferDesc.DepthOrArraySize = 1;
			vertexBufferDesc.MipLevels = 1;
			vertexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
			vertexBufferDesc.SampleDesc.Count = 1;
			vertexBufferDesc.SampleDesc.Quality = 0;
			vertexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			vertexBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			ThrowIfFailed(aDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE,
				&vertexBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&myVertexBufferGPU)));
			ThrowIfFailed(myVertexBufferGPU->SetName(L"myVertexBufferGPU"));

			ThrowIfFailed(myVertexBufferGPU->Map(0, &range, &pDataBegin));
			std::memcpy(pDataBegin, aVertexBuffer, aVertexBufferByteSize);
			myVertexBufferGPU->Unmap(0, nullptr);

			myVertexBufferView.BufferLocation = myVertexBufferGPU->GetGPUVirtualAddress();
			myVertexBufferView.SizeInBytes = myVertexBufferByteSize;
			myVertexBufferView.StrideInBytes = myVertexByteStride;
		}
		catch (...)
		{
			std::throw_with_nested(std::runtime_error("failed to create vertex buffer"));
		}
		try
		{
			D3D12_RESOURCE_DESC indexBufferDesc{};
			indexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			indexBufferDesc.Alignment = 0;
			indexBufferDesc.Width = aIndexBufferByteSize;
			indexBufferDesc.Height = 1;
			indexBufferDesc.DepthOrArraySize = 1;
			indexBufferDesc.MipLevels = 1;
			indexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
			indexBufferDesc.SampleDesc.Count = 1;
			indexBufferDesc.SampleDesc.Quality = 0;
			indexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			indexBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			ThrowIfFailed(aDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE,
				&indexBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&myIndexBufferGPU)));
			ThrowIfFailed(myIndexBufferGPU->SetName(L"myIndexBufferGPU"));

			ThrowIfFailed(myIndexBufferGPU->Map(0, &range, &pDataBegin));
			std::memcpy(pDataBegin, aIndexBuffer, aIndexBufferByteSize);
			myIndexBufferGPU->Unmap(0, nullptr);

			static_assert(sizeof(Index) == sizeof(unsigned short), "DXGI_FORMAT_R16_UINT does not match underlying type");
			myIndexBufferView.BufferLocation = myIndexBufferGPU->GetGPUVirtualAddress();
			myIndexBufferView.SizeInBytes = myIndexBufferByteSize;
			myIndexBufferView.Format = DXGI_FORMAT_R16_UINT;
		}
		catch (...)
		{
			std::throw_with_nested(std::runtime_error("failed to create index buffer"));
		}
	}
	catch (...)
	{
		std::throw_with_nested(std::runtime_error("failed to initialize mesh"));
	}
}

const D3D12_VERTEX_BUFFER_VIEW& Mesh::VertexBufferView() const
{
	return myVertexBufferView;
}

const D3D12_INDEX_BUFFER_VIEW& Mesh::IndexBufferView() const
{
	return myIndexBufferView;
}