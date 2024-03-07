#pragma once
#include "pch.h"
#include "GraphicsEngine/Vertex.h"
#include "GraphicsEngine/Buffer.h"

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
	void Init(std::wstring_view aName, Buffer&& aVertexBuffer, Buffer&& aIndexBuffer);
	const D3D12_VERTEX_BUFFER_VIEW& VertexBufferView() const;
	const D3D12_INDEX_BUFFER_VIEW& IndexBufferView() const;
private:
	std::wstring myName;
	Buffer myVertexBuffer;
	Buffer myIndexBuffer;
	D3D12_VERTEX_BUFFER_VIEW myVertexBufferView{};
	D3D12_INDEX_BUFFER_VIEW myIndexBufferView{};
};