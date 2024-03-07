#include "pch.h"
#include "GraphicsEngine/Mesh.h"
#include "GraphicsEngine/Vertex.h"

void Mesh::Init(std::wstring_view aName, Buffer&& aVertexBuffer, Buffer&& aIndexBuffer)
{
		myName = aName;
		myVertexBuffer = std::move(aVertexBuffer);
		myIndexBuffer = std::move(aIndexBuffer);

		myVertexBufferView.BufferLocation = myVertexBuffer.GetGPUVirtualAddress();
		myVertexBufferView.SizeInBytes = myVertexBuffer.SizeInBytes();
		myVertexBufferView.StrideInBytes = myVertexBuffer.StrideInBytes();

		static_assert(sizeof(Index) == sizeof(unsigned short), "DXGI_FORMAT_R16_UINT does not match underlying type");
		myIndexBufferView.BufferLocation = myIndexBuffer.GetGPUVirtualAddress();
		myIndexBufferView.SizeInBytes = myIndexBuffer.SizeInBytes();
		myIndexBufferView.Format = DXGI_FORMAT_R16_UINT;
}

const D3D12_VERTEX_BUFFER_VIEW& Mesh::VertexBufferView() const
{
	return myVertexBufferView;
}

const D3D12_INDEX_BUFFER_VIEW& Mesh::IndexBufferView() const
{
	return myIndexBufferView;
}