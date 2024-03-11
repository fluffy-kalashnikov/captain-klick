#include "GraphicsDevice.h"
#include "Window.h"

void GraphicsDevice::Initialize()
{
    InitDevice();
    InitPipeline();
}

void GraphicsDevice::InitDevice()
{
    {
        ComPtr<ID3D12Debug> debug;
        ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)));
        ThrowIfFailed(debug.As(&myDebug));
        myDebug->EnableDebugLayer();
    }

    ThrowIfFailed(CreateDXGIFactory(IID_PPV_ARGS(&myFactory)));
    ThrowIfFailed(myFactory->EnumAdapters(0, &myAdapter));
    ThrowIfFailed(D3D12CreateDevice(myAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&myDevice)));
    myDevice->SetName(L"myDevice");

    ThrowIfFailed(myDevice->QueryInterface(IID_PPV_ARGS(&myInfoQueue)));
    ThrowIfFailed(myInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true));
    ThrowIfFailed(myInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true));
    ThrowIfFailed(myInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true));

    //command queue
    {
        D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
        commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        ThrowIfFailed(myDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&myCommandQueue)));
        ThrowIfFailed(myCommandQueue->SetName(L"myCommandQueue"));
    }

    //swap chain
    {
        DXGI_SWAP_CHAIN_DESC swapChainDesc{};
        swapChainDesc.BufferCount = FRAME_COUNT;
        swapChainDesc.BufferDesc.Width = static_cast<UINT>(globalWindow.size.x);
        swapChainDesc.BufferDesc.Height = static_cast<UINT>(globalWindow.size.y);
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.OutputWindow = globalWindow.hwnd;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.Windowed = TRUE;

        ComPtr<IDXGISwapChain> swapChain;
        ThrowIfFailed(myFactory->CreateSwapChain(myCommandQueue.Get(), &swapChainDesc, &swapChain));
        ThrowIfFailed(swapChain.As(&mySwapChain));
    }
    ThrowIfFailed(myFactory->MakeWindowAssociation(globalWindow.hwnd, DXGI_MWA_NO_ALT_ENTER));
    myFrameIndex = mySwapChain->GetCurrentBackBufferIndex();


    //cbv srv dsv rtv heaps
    {
        myCbvSrvDescriptorSize = myDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        myDsvDescriptorSize = myDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        myRtvDescriptorSize = myDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        D3D12_DESCRIPTOR_HEAP_DESC cbvSrvHeapDesc{};
        cbvSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        cbvSrvHeapDesc.NumDescriptors = 512; //TODO: figure out actual number of shite required
        cbvSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(myDevice->CreateDescriptorHeap(&cbvSrvHeapDesc, IID_PPV_ARGS(&myCbvSrvHeap)));
        ThrowIfFailed(myCbvSrvHeap->SetName(L"myCbvSrvHeap"));

        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(myDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&myDsvHeap)));
        ThrowIfFailed(myDsvHeap->SetName(L"myDsvHeap"));

        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
        rtvHeapDesc.NumDescriptors = FRAME_COUNT;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(myDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&myRtvHeap)));
        ThrowIfFailed(myRtvHeap->SetName(L"myRtvHeap"));
    }
    
    //render targets
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(myRtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT n = 0; n < FRAME_COUNT; n++)
    {
        ThrowIfFailed(mySwapChain->GetBuffer(n, IID_PPV_ARGS(&myBackBuffers[n])));
        myDevice->CreateRenderTargetView(myBackBuffers[n].Get(), nullptr, rtvHeapHandle);
        ThrowIfFailed(myBackBuffers[n]->SetName(L"myBackBuffers[x]"));

        rtvHeapHandle.Offset(1, myRtvDescriptorSize);
    }
    


    //fence
    ThrowIfFailed(myDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&myFence)));
    ThrowIfFailed(myFence->SetName(L"myFence"));
    myFenceValue = 1;
    myFenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (myFenceEvent == nullptr)
    {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
    }


    //command allocator/list
    ThrowIfFailed(myDevice->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&myCommandAllocator)));
    ThrowIfFailed(myCommandAllocator->SetName(L"myCommandAllocator"));

    ThrowIfFailed(myDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
        myCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&myCommandList)));
    ThrowIfFailed(myCommandList->SetName(L"myCommandList"));


    //depth stencil
    CD3DX12_HEAP_PROPERTIES defaultHeap(D3D12_HEAP_TYPE_DEFAULT);

    D3D12_RESOURCE_DESC depthStencilDesc{};
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = static_cast<UINT64>(globalWindow.size.x);
    depthStencilDesc.Height = static_cast<UINT>(globalWindow.size.y);
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE depthStencilClearValue{};
    depthStencilClearValue.DepthStencil.Depth = 1.f;
    depthStencilClearValue.DepthStencil.Stencil = 0;
    depthStencilClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    ThrowIfFailed(myDevice->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE,
        &depthStencilDesc, D3D12_RESOURCE_STATE_COMMON, &depthStencilClearValue, IID_PPV_ARGS(&myDepthStencilBuffer)));
    ThrowIfFailed(myDepthStencilBuffer->SetName(L"myDepthStencilBuffer"));

    myDevice->CreateDepthStencilView(myDepthStencilBuffer.Get(),
        nullptr, myDsvHeap->GetCPUDescriptorHandleForHeapStart());

    D3D12_RESOURCE_BARRIER depthStencilBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        myDepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    myCommandList->ResourceBarrier(1, &depthStencilBarrier);
}

void GraphicsDevice::InitPipeline()
{
    {
        D3D12_ROOT_PARAMETER rootParameters[2]{};
        {
            D3D12_ROOT_PARAMETER& instanceBuffer = rootParameters[0];
            instanceBuffer.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            instanceBuffer.Descriptor.ShaderRegister = 0; //TODO: set to actual constant buffer descriptor
            instanceBuffer.Descriptor.RegisterSpace = 0;
            instanceBuffer.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

            D3D12_ROOT_PARAMETER& passBuffer = rootParameters[1];
            passBuffer.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            passBuffer.Descriptor.ShaderRegister = 1; //TODO: set to actual constant buffer descriptor
            passBuffer.Descriptor.RegisterSpace = 0;
            passBuffer.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        }

        D3D12_STATIC_SAMPLER_DESC staticSamplerDescs[1]{};
        {
            D3D12_STATIC_SAMPLER_DESC& trilinearWrap = staticSamplerDescs[0];
            trilinearWrap.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            trilinearWrap.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            trilinearWrap.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            trilinearWrap.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            trilinearWrap.MipLODBias = 0;
            trilinearWrap.MaxAnisotropy = 1;
            trilinearWrap.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
            trilinearWrap.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
            trilinearWrap.MinLOD = -D3D12_FLOAT32_MAX;
            trilinearWrap.MaxLOD = D3D12_FLOAT32_MAX;
            trilinearWrap.ShaderRegister = 0;
            trilinearWrap.RegisterSpace = 0;
            trilinearWrap.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        }

        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
        rootSignatureDesc.NumParameters = sizeof(rootParameters) / sizeof(rootParameters[0]);
        rootSignatureDesc.pParameters = rootParameters;
        rootSignatureDesc.NumStaticSamplers = sizeof(staticSamplerDescs) / sizeof(staticSamplerDescs[0]);
        rootSignatureDesc.pStaticSamplers = staticSamplerDescs;
        rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;

        ComPtr<ID3DBlob> rootSignature, rootSignatureError;
        ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc,
            D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSignature, &rootSignatureError));

        ThrowIfFailed(myDevice->CreateRootSignature(0, rootSignature->GetBufferPointer(),
            rootSignature->GetBufferSize(), IID_PPV_ARGS(&myRootSignature)));
        ThrowIfFailed(myRootSignature->SetName(L"myRootSignature"));
    }



    {
        ComPtr<ID3DBlob> defaultVsBlob, defaultPsBlob;
        ThrowIfFailed(D3DReadFileToBlob(L"Model_VS.cso", &defaultVsBlob));
        ThrowIfFailed(D3DReadFileToBlob(L"Model_PS.cso", &defaultPsBlob));

        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        D3D12_RASTERIZER_DESC rasterizerDesc{};
        rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
        rasterizerDesc.FrontCounterClockwise = FALSE;
        rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        rasterizerDesc.DepthClipEnable = TRUE;
        rasterizerDesc.MultisampleEnable = FALSE;
        rasterizerDesc.AntialiasedLineEnable = FALSE;
        rasterizerDesc.ForcedSampleCount = 0;
        rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        D3D12_BLEND_DESC noBlendDesc{};
        noBlendDesc.AlphaToCoverageEnable = FALSE;
        noBlendDesc.IndependentBlendEnable = FALSE;
        noBlendDesc.RenderTarget[0].BlendEnable = FALSE;
        noBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
        noBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
        noBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
        noBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        noBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
        noBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
        noBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        noBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
        noBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
        depthStencilDesc.DepthEnable = TRUE;
        depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        depthStencilDesc.StencilEnable = FALSE;
        depthStencilDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
        depthStencilDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
        depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
        depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        depthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        
        D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
        graphicsPipelineStateDesc.pRootSignature = myRootSignature.Get();
        graphicsPipelineStateDesc.VS = CD3DX12_SHADER_BYTECODE(defaultVsBlob->GetBufferPointer(), defaultVsBlob->GetBufferSize());
        graphicsPipelineStateDesc.PS = CD3DX12_SHADER_BYTECODE(defaultPsBlob->GetBufferPointer(), defaultPsBlob->GetBufferSize());
        graphicsPipelineStateDesc.BlendState = noBlendDesc;
        graphicsPipelineStateDesc.SampleMask = UINT_MAX;
        graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
        graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
        graphicsPipelineStateDesc.InputLayout = { inputElementDescs, sizeof(inputElementDescs) / sizeof(inputElementDescs[0]) };
        graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        graphicsPipelineStateDesc.NumRenderTargets = 1;
        graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        graphicsPipelineStateDesc.SampleDesc.Count = 1;
        graphicsPipelineStateDesc.SampleDesc.Quality = 0;

        ThrowIfFailed(myDevice->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&myCubePipelineState)));
        ThrowIfFailed(myCubePipelineState->SetName(L"myCubePipelineState"));
    }
}

void GraphicsDevice::Shutdown()
{
    // Wait for the GPU to be done with all resources.
    WaitForGPU();
    CloseHandle(myFenceEvent);
}

ComPtr<ID3D12GraphicsCommandList> GraphicsDevice::BeginFrame()
{
    if (globalWindow.resize != Vec2::Zero)
    {
        myScissorRect.right = static_cast<LONG>(globalWindow.resize.x);
        myScissorRect.bottom = static_cast<LONG>(globalWindow.resize.y);

        myViewport.Width = globalWindow.resize.x;
        myViewport.Height = globalWindow.resize.y;
        myViewport.MinDepth = 0;
        myViewport.MaxDepth = 1;
    }

    myCommandList->SetGraphicsRootSignature(myRootSignature.Get());
    myCommandList->SetPipelineState(myCubePipelineState.Get());
    myCommandList->RSSetScissorRects(1, &myScissorRect);
    myCommandList->RSSetViewports(1, &myViewport);

    {
        D3D12_RESOURCE_BARRIER rtvResourceBarrier{};
        rtvResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        rtvResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        rtvResourceBarrier.Transition.pResource = myBackBuffers[myFrameIndex].Get();
        rtvResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        rtvResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        rtvResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        myCommandList->ResourceBarrier(1, &rtvResourceBarrier);
    }


    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHeapHandle(myDsvHeap->GetCPUDescriptorHandleForHeapStart());
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(myRtvHeap->GetCPUDescriptorHandleForHeapStart());
    rtvHeapHandle.Offset(myFrameIndex, myRtvDescriptorSize);
    myCommandList->OMSetRenderTargets(1, &rtvHeapHandle, FALSE, &dsvHeapHandle);

    const float clearColor[4] = { 0.f, 0.2f, 0.4f, 1.f };
    myCommandList->ClearRenderTargetView(rtvHeapHandle, clearColor, 0, nullptr);
    myCommandList->ClearDepthStencilView(dsvHeapHandle, 
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1, 0, 0, nullptr);
    return std::move(myCommandList);
}

void GraphicsDevice::EndFrame(ComPtr<ID3D12GraphicsCommandList>&& aCommandList)
{
    myCommandList = std::move(aCommandList);
    if (myCommandList == nullptr)
    {
        throw std::runtime_error("command list unavailable");
    }
        
    D3D12_RESOURCE_BARRIER rtvResourceBarrier{};
    rtvResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    rtvResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    rtvResourceBarrier.Transition.pResource = myBackBuffers[myFrameIndex].Get();
    rtvResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    rtvResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    rtvResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    myCommandList->ResourceBarrier(1, &rtvResourceBarrier);
    ThrowIfFailed(myCommandList->Close());


    ID3D12CommandList* ppCommandLists[] = { myCommandList.Get() };
    myCommandQueue->ExecuteCommandLists(sizeof(ppCommandLists) / sizeof(ppCommandLists[0]), ppCommandLists);

    ThrowIfFailed(mySwapChain->Present(1, 0));
    WaitForGPU();
    ThrowIfFailed(myCommandAllocator->Reset());
    ThrowIfFailed(myCommandList->Reset(myCommandAllocator.Get(), myCubePipelineState.Get()));
}

Buffer GraphicsDevice::CreateDefaultBuffer(const std::wstring& aName, 
    const void* aBuffer, const UINT aBufferByteSize, const UINT aBufferByteStride)
{
    if (myCommandList == nullptr)
    {
        throw std::runtime_error("command list unavailable");
    }

    ComPtr<ID3DBlob> blobCPU;
    ThrowIfFailed(D3DCreateBlob(aBufferByteSize, &blobCPU));
    std::memcpy(blobCPU->GetBufferPointer(), aBuffer, blobCPU->GetBufferSize());



    D3D12_HEAP_PROPERTIES defaultHeap{}, uploadHeap{};
    defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;
    uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC bufferDesc{};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    bufferDesc.Width = aBufferByteSize;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.SampleDesc.Quality = 0;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ComPtr<ID3D12Resource> blobGPU, blobUpload;
    ThrowIfFailed(myDevice->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE,
        &bufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&blobGPU)));
    ThrowIfFailed(blobGPU->SetName(aName.c_str()));

    ThrowIfFailed(myDevice->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE,
        &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&blobUpload)));
    ThrowIfFailed(blobUpload->SetName((aName + L" upload").c_str()));


    D3D12_RANGE range{ 0, 0 };
    void* pData = nullptr;
    blobUpload->Map(0, &range, &pData);
    std::memcpy(pData, aBuffer, aBufferByteSize);
    blobUpload->Unmap(0, nullptr);

    {
        D3D12_RESOURCE_BARRIER commonCopyDest = CD3DX12_RESOURCE_BARRIER::Transition(
            blobGPU.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
        D3D12_RESOURCE_BARRIER copyDestGenericRead = CD3DX12_RESOURCE_BARRIER::Transition(
            blobGPU.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
            
        myCommandList->ResourceBarrier(1, &commonCopyDest);
        myCommandList->CopyResource(blobGPU.Get(), blobUpload.Get());
        myCommandList->ResourceBarrier(1, &copyDestGenericRead);
    }

    Buffer buffer;
    buffer.myBlobCPU = std::move(blobCPU);
    buffer.myBlobGPU = std::move(blobGPU);
    buffer.myBlobUpload = std::move(blobUpload);
    buffer.myBlobByteSize = aBufferByteSize;
    buffer.myBlobByteStride = aBufferByteStride;
    return buffer;
}

Buffer GraphicsDevice::CreateUploadBuffer(const std::wstring& aName, 
    const UINT aBufferByteSize, const UINT aBufferByteStride)
{
    D3D12_HEAP_PROPERTIES uploadHeap{};
    uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC bufferDesc{};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    bufferDesc.Width = aBufferByteSize;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.SampleDesc.Quality = 0;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ComPtr<ID3D12Resource> bufferGPU;
    ThrowIfFailed(myDevice->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE,
        &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&bufferGPU)));
    ThrowIfFailed(bufferGPU->SetName(aName.c_str()));

    Buffer buffer;
    buffer.myBlobCPU = nullptr;
    buffer.myBlobGPU = std::move(bufferGPU);
    buffer.myBlobUpload = nullptr;
    buffer.myBlobByteSize = aBufferByteSize;
    buffer.myBlobByteStride = aBufferByteStride;
    return buffer;
}

void GraphicsDevice::WaitForGPU()
{
    // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. More advanced samples 
    // illustrate how to use fences for efficient resource usage.

    // Signal and increment the fence value.
    const UINT64 fence = myFenceValue;
    ThrowIfFailed(myCommandQueue->Signal(myFence.Get(), fence));
    myFenceValue++;

    // Wait until the previous frame is finished.
    if (myFence->GetCompletedValue() < fence)
    {
        ThrowIfFailed(myFence->SetEventOnCompletion(fence, myFenceEvent));
        ::WaitForSingleObject(myFenceEvent, INFINITE);
    }

    myFrameIndex = mySwapChain->GetCurrentBackBufferIndex();
}