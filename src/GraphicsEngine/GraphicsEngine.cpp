#include "pch.h"
#include "GraphicsEngine.h"
#include "HLSL/ConstantBuffers.hlsli.h"
#include "Window.h"
#include "Vertex.h"



void GraphicsEngine::OnInitialize()
{
    myGraphicsDevice.Initialize();
    myPassConstantBuffer = myGraphicsDevice.CreateUploadBuffer<cbPassStruct>(L"cbPass");

    const std::tuple<Vec3, Vec3, Vec3, Vec3> cubes[] = {
        /** rgb */
        { { 0, -50, 0 }, { 0, 0, 0 }, { 20, 1, 20 }, { 0.3f, 0.3f, 0.33f } },
        { { -200, 50, -400 }, { 0, 0, 0 }, { 1, 1, 1 }, { 0.9f, 0.2f, 0.2f } },
        { { -200, 50, 400 }, { 0, 0, 0 }, { 1, 1, 1 }, { 0.2f, 0.9f, 0.2f } },
        { { 200, 50, 400 }, { 0, 0, 0 }, { 1, 1, 1 }, { 0.2f, 0.2f, 0.9f } },
        /** ycp */
        { { 200, 50, -400 }, { 0, 0, 0 }, { 1, 1, 1 }, { 0.9f, 0.9f, 0.2f } },
        { { 200, 50, 0 }, { 0, 0, 0 }, { 1, 1, 1 }, { 0.2f, 0.9f, 0.9f } },
        { { -200, 50, 0 }, { 0, 0, 0 }, { 1, 1, 1 }, { 0.9f, 0.2f, 0.9f } },
        /** CORNER */
        { { -600, 100, -600 }, { 0, 0, 0 }, { 2, 2, 2 }, { 1, 1, 1 } },
        { { -600, 100, 600 }, { 0, 0, 0 }, { 2, 2, 2 }, { 0, 0, 0 } },
        { { 600, 100, 600 }, { 0, 0, 0 }, { 2, 2, 2 }, { 1, 1, 1 } },
        { { 600, 100, -600 }, { 0, 0, 0 }, { 2, 2, 2 }, { 0, 0, 0 } },
    };

    for (const auto&[location, rotation, scale, color] : cubes)
    {
        const Vec4 color4 = Vec4(color.x, color.y, color.z, 1);
        const Mat4 transform = Mat4::TransformMatrix(location, rotation, scale);

        Buffer& buffer = myInstanceConstantBuffers.emplace_back();
        buffer = myGraphicsDevice.CreateDefaultBuffer<cbInstanceStruct>(L"cbInstance", { color4, transform });
    }


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
        10, 11, 8,
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
    cbPassStruct pass{};
    pass.VP = aToViewMatrix * aToProjectionMatrix;
    pass.timeSeconds = aTimeSeconds;
    pass.deltaSeconds = aDeltaSeconds;
    myPassConstantBuffer.Upload(pass);

    ComPtr<ID3D12GraphicsCommandList> commandList = myGraphicsDevice.BeginFrame();
        
    commandList->SetGraphicsRootConstantBufferView(1, myPassConstantBuffer.GetGPUVirtualAddress());


    for (Buffer& instanceConstantBuffer : myInstanceConstantBuffers)
    {
        commandList->SetGraphicsRootConstantBufferView(0, instanceConstantBuffer.GetGPUVirtualAddress());
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->IASetVertexBuffers(0, 1, &myMesh.VertexBufferView());
        commandList->IASetIndexBuffer(&myMesh.IndexBufferView());
        commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);
    }

    myGraphicsDevice.EndFrame(std::move(commandList));
}

void GraphicsEngine::OnDestroy()
{
    myGraphicsDevice.Shutdown();
}