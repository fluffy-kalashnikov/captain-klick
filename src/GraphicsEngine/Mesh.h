#pragma once
#include "pch.h"
#include "Vertex.h"

struct ID3D12Resource;
struct ID3D12Device;
class Mesh
{
public:
	Mesh() = default;
	Mesh(const Mesh&) = delete;
	Mesh(Mesh&&) = default;
	Mesh& operator=(const Mesh&) = delete;
	Mesh& operator=(Mesh&&) = default;
	~Mesh() = default;
	template<class T, std::size_t VERTEX_COUNT, std::size_t INDEX_COUNT>
	void Init(ID3D12Device* aDevice, std::string_view aName, 
		const T(&aVertexBuffer)[VERTEX_COUNT], const Index(&aIndexBuffer)[INDEX_COUNT]);
	const D3D12_VERTEX_BUFFER_VIEW& VertexBufferView() const;
	const D3D12_INDEX_BUFFER_VIEW& IndexBufferView() const;
private:
	std::string myName;
	ComPtr<ID3D12Resource> myVertexBufferGPU;
	ComPtr<ID3D12Resource> myIndexBufferGPU;
	ComPtr<ID3D12Resource> myVertexBufferUploader;
	ComPtr<ID3D12Resource> myIndexBufferUploader;
	UINT myVertexByteStride{};
	UINT myVertexBufferByteSize{};
	UINT myIndexBufferByteSize{};
	D3D12_VERTEX_BUFFER_VIEW myVertexBufferView{};
	D3D12_INDEX_BUFFER_VIEW myIndexBufferView{};
	void Init(ID3D12Device* aDevice, std::string_view aName, UINT aVertexByteStride, const void* aVertexBuffer, 
		UINT aVertexBufferByteSize, const void* aIndexBuffer, UINT aIndexBufferByteStride);
};

template<class T, std::size_t VERTEX_COUNT, std::size_t INDEX_COUNT>
void Mesh::Init(ID3D12Device* aDevice, std::string_view aName,
	const T(&aVertexBuffer)[VERTEX_COUNT], const Index(&aIndexBuffer)[INDEX_COUNT])
{
	Init(aDevice, aName, sizeof(aVertexBuffer[0]), aVertexBuffer, sizeof(aVertexBuffer), aIndexBuffer, sizeof(aIndexBuffer));
}