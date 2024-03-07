#include "pch.h"
#include "GraphicsEngine.h"
#include "HLSL/ConstantBuffers.hlsli.h"
#include "Window.h"
#include "Vertex.h"



void GraphicsEngine::OnInitialize()
{
    myGraphicsDevice.Initialize();
    myInstanceConstantBuffer = myGraphicsDevice.CreateUploadBuffer<cbInstanceStruct>(L"cbInstance");
    myPassConstantBuffer = myGraphicsDevice.CreateUploadBuffer<cbPassStruct>(L"cbPass");

    const Vertex vertices[24] = {
        //top
        { -50, 50, 50 },
        {  50, 50, 50 },
        {  50, 50,-50 },
        { -50, 50,-50 },
        //right
        { 50, 50,-50 },
        { 50, 50, 50 },
        { 50,-50, 50 },
        { 50,-50,-50 },
        //front
        { -50, 50,-50 },
        {  50, 50,-50 },
        {  50,-50,-50 },
        { -50,-50,-50 },
        //left
        { -50, 50, 50 },
        { -50, 50,-50 },
        { -50,-50,-50 },
        { -50,-50, 50 },
        //back
        {  50, 50, 50 },
        { -50, 50, 50 },
        { -50,-50, 50 },
        {  50,-50, 50 },
        //bottom
        {  50,-50,-50 },
        {  50,-50, 50 },
        { -50,-50, 50 },
        { -50,-50,-50 }
    };
    const Index indices[36] = {
        0, 1, 2,
        2, 3, 0,
        4, 5, 6,
        6, 7, 4,
        8, 9, 10,
        10, 11,  8,
        12, 13, 14,
        14, 15, 12,
        16, 17, 18,
        18, 19, 16,
        20, 21, 22,
        22, 23, 20
    };
    Buffer vertexBuffer, indexBuffer;
    indexBuffer = myGraphicsDevice.CreateDefaultBuffer(L"cube index buffer", indices);
    vertexBuffer = myGraphicsDevice.CreateDefaultBuffer(L"cube vertex buffer", vertices);
    myMesh.Init(L"cube mesh", std::move(vertexBuffer), std::move(indexBuffer));

    myGraphicsDevice.WaitForGPU();
}

void GraphicsEngine::OnUpdate(float aDeltaSeconds, float aTimeSeconds, const Mat4& aToViewMatrix, const Mat4& aToProjectionMatrix)
{
    cbInstanceStruct instance{};
    instance.transform = Mat4::TranslationMatrix(0.f, 0.f, 200);
    myInstanceConstantBuffer.Upload(instance);

    cbPassStruct pass{};
    pass.VP = aToViewMatrix * aToProjectionMatrix;
    pass.timeSeconds = aTimeSeconds;
    pass.deltaSeconds = aDeltaSeconds;
    myPassConstantBuffer.Upload(pass);

    
    ComPtr<ID3D12GraphicsCommandList> commandList = myGraphicsDevice.BeginFrame();
        
    commandList->SetGraphicsRootConstantBufferView(0, myInstanceConstantBuffer.GetGPUVirtualAddress());
    commandList->SetGraphicsRootConstantBufferView(1, myPassConstantBuffer.GetGPUVirtualAddress());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &myMesh.VertexBufferView());
    commandList->IASetIndexBuffer(&myMesh.IndexBufferView());
    commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

    myGraphicsDevice.EndFrame(std::move(commandList));
}

void GraphicsEngine::OnDestroy()
{
    myGraphicsDevice.Shutdown();
}