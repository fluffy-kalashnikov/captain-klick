#include "pch.h"
#include "GraphicsEngine.h"
#include "GraphicsGlobals.h"
#include "HLSL/includeConstantBuffers.hlsli.h"
#include "InputHandler.h"
#include "Vertex.h"
#include "TimerScope.h"

using namespace GraphicsGlobals;


void GraphicsEngine::Initialize(GraphicsDevice* aDevice)
{
    myDevice = aDevice;
    myDevice->Initialize();
    myCommandAllocator = myDevice->CreateCommandAllocator(L"GraphicsEngine::myCommandAllocator");
    myCommandList = myDevice->CreateGraphicsCommandList(L"GrapihcsEngine::myCommandList", nullptr, myCommandAllocator.Get());

    D3D12_DESCRIPTOR_HEAP_DESC cbvSrvHeapDesc{};
    cbvSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvSrvHeapDesc.NumDescriptors = 512;
    cbvSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    myCbvSrvHeap = myDevice->CreateDescriptorHeap(L"GraphicsEngine::myCbvSrvHeap", cbvSrvHeapDesc);

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    myDsvHeap = myDevice->CreateDescriptorHeap(L"GraphicsEngine::myDsvHeap", dsvHeapDesc);

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
    rtvHeapDesc.NumDescriptors = 1;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    myRtvHeap = myDevice->CreateDescriptorHeap(L"GraphicsEngine::myRtvHeap", rtvHeapDesc);


    myQueue.Initialize(myDevice, L"GraphicsEngine::myQueue");
    GraphicsGlobals::Create(aDevice, myCommandList.Get());

    //mySwapChain.Initialize(myDevice, &myQueue);



    myPassConstantBuffer = myDevice->CreateUploadBuffer<cbPassStruct>(L"cbPass");

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
        buffer = myDevice->CreateDefaultBuffer<cbInstanceStruct>(L"cbInstance", myCommandList.Get(), { color4, transform });
    }
}

void GraphicsEngine::Update(const GraphicsSwapChain* aSwapChain, const Camera aCamera) //TODO: handle data races
{
    if (aSwapChain->GetHeight() < 4 && aSwapChain->GetWidth() < 4)
    {
        return;
    }
    if (aSwapChain->IsResizing())
    {
        const UINT width = aSwapChain->GetWidth(),
                   height = aSwapChain->GetHeight();

        myScissorRect.right = width;
        myScissorRect.bottom = height;
        myViewport.Width = static_cast<float>(width);
        myViewport.Height = static_cast<float>(height);
        myViewport.MinDepth = 0;
        myViewport.MaxDepth = 1;
        Resize(width, height);
    }

    cbPassStruct pass{}; //TODO: separate rotation and translation matrix
    pass.cameraV = aCamera.GetViewMatrix();
    pass.cameraP = aCamera.GetProjectionMatrix();
    pass.cameraVP = pass.cameraV * pass.cameraP;
    myPassConstantBuffer.Upload(pass);

    D3D12_CPU_DESCRIPTOR_HANDLE rtv = myRtvHeap->GetCPUDescriptorHandleForHeapStart(),
                                dsv = myDsvHeap->GetCPUDescriptorHandleForHeapStart();
    const float clearColor[4] = { 0.f, 0.f, 0.f, 0.f };
    myCommandList->RSSetScissorRects(1, &myScissorRect);
    myCommandList->RSSetViewports(1, &myViewport);
    myCommandList->ClearRenderTargetView(myRtvHeap->GetCPUDescriptorHandleForHeapStart(), clearColor, 0, nullptr);
    myCommandList->ClearDepthStencilView(myDsvHeap->GetCPUDescriptorHandleForHeapStart(),
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, nullptr);
    myCommandList->SetGraphicsRootSignature(globalRootSignature.Get());
    myCommandList->SetPipelineState(globalWorkPipeline.Get());
    myCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    myCommandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
    for (int n = 0; n < 20000; n++)
    {
        myCommandList->DrawInstanced(4, 1, 0, 0);
    }

    myCommandList->SetPipelineState(globalModelPipeline.Get());
    myCommandList->SetGraphicsRootConstantBufferView(1, myPassConstantBuffer.GetGPUVirtualAddress());
    for (Buffer& instanceConstantBuffer : myInstanceConstantBuffers)
    {
        myCommandList->SetGraphicsRootConstantBufferView(0, instanceConstantBuffer.GetGPUVirtualAddress());
        myCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        myCommandList->IASetVertexBuffers(0, 1, &globalCubeMesh.VertexBufferView());
        myCommandList->IASetIndexBuffer(&globalCubeMesh.IndexBufferView());
        myCommandList->DrawIndexedInstanced(36, 1, 0, 0, 0);
    }



    ThrowIfFailed(myCommandList->Close());
    myQueue.ExecuteCommandList(myCommandList);
    {
        TimerScope timer("GraphicsEngine");
        myQueue.WaitForIdle();
    }
    ThrowIfFailed(myCommandAllocator->Reset());
    ThrowIfFailed(myCommandList->Reset(myCommandAllocator.Get(), nullptr));
  
    //TODO: indicate to swapchain that the texture has been rendered and that we want to switch which texture is used for display
}

void GraphicsEngine::Resize(const UINT aWidth, const UINT aHeight)
{
    if (aWidth != 0 && aHeight != 0)
    {
        mySceneBuffer = myDevice->CreateTexture2D(L"GraphicsEngine::mySceneBuffer", aWidth, aHeight);
        myDepthStencilBuffer = myDevice->CreateTexture2D(L"GraphicsEngine::myDepthStencilBuffer", aWidth, aHeight,
            DXGI_FORMAT_D32_FLOAT, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

        myDevice->CreateRenderTargetView(mySceneBuffer.Get(), myRtvHeap->GetCPUDescriptorHandleForHeapStart());
        myDevice->CreateDepthStencilView(myDepthStencilBuffer.Get(), myDsvHeap->GetCPUDescriptorHandleForHeapStart());
    }
}

void GraphicsEngine::Shutdown()
{
    myQueue.Signal();
    myQueue.WaitForIdle();
    GraphicsGlobals::Destroy();
}

GraphicsQueue& GraphicsEngine::GetGraphicsQueue()
{
    return myQueue;
}