#include "pch.h"
#include "GraphicsEngine.h"
#include "HLSL/ConstantBuffers.hlsli.h"
#include "Window.h"
#include "Vertex.h"

/*
TODO
    - [x] perspective matrices
    - [x] transform matrices
    - [ ] cube geometry vertex buffer
    - [ ] index buffer
    - [ ] draw indexed
*/



void GraphicsEngine::OnInitialize()
{
    try
    {
        LoadPipeline();
        LoadAssets();
    }
    catch (...)
    {
        std::throw_with_nested(std::runtime_error("graphics engine failed to initialize"));
    }
}

void GraphicsEngine::LoadPipeline()
{
    try
    {
        {
            ComPtr<ID3D12Debug> debug;
            ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)));
            ThrowIfFailed(debug.As(&myDebug));
            myDebug->EnableDebugLayer();
            //myDebug->SetEnableGPUBasedValidation(true);
        }
        
        ThrowIfFailed(CreateDXGIFactory(IID_PPV_ARGS(&myFactory)));
        ThrowIfFailed(myFactory->EnumAdapters(0, &myAdapter));
        ThrowIfFailed(D3D12CreateDevice(myAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&myDevice)));
        myDevice->SetName(L"myDevice");
        myCbvSrvDescriptorSize = myDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        myDsvDescriptorSize = myDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        myRtvDescriptorSize = myDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        {
            D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
            commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            ThrowIfFailed(myDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&myCommandQueue)));
            ThrowIfFailed(myCommandQueue->SetName(L"myCommandQueue"));
        }
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


        {
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
        {
            D3D12_HEAP_PROPERTIES heapProperties{};
            heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

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

            D3D12_CLEAR_VALUE clearValue{};
            clearValue.DepthStencil.Depth = 1.f;
            clearValue.DepthStencil.Stencil = 0;
            clearValue.Format = DXGI_FORMAT_D32_FLOAT;
            ThrowIfFailed(myDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE,
                &depthStencilDesc, D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_PPV_ARGS(&myDepthStencilBuffer)));
            ThrowIfFailed(myDepthStencilBuffer->SetName(L"myDepthStencilBuffer"));

            myDevice->CreateDepthStencilView(myDepthStencilBuffer.Get(), nullptr, DepthStencilView());
        }
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = myRtvHeap->GetCPUDescriptorHandleForHeapStart();
            for (UINT n = 0; n < FRAME_COUNT; n++)
            {
                ThrowIfFailed(mySwapChain->GetBuffer(n, IID_PPV_ARGS(&myBackBuffers[n])));
                myDevice->CreateRenderTargetView(myBackBuffers[n].Get(), nullptr, rtvHandle);
                rtvHandle.ptr += myRtvDescriptorSize;

                ThrowIfFailed(myBackBuffers[n]->SetName(L"myBackBuffers[x]"));
            }
        }


        ThrowIfFailed(myDevice->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&myCommandAllocator)));
        ThrowIfFailed(myCommandAllocator->SetName(L"myCommandAllocator"));
    }
    catch (...)
    {
        std::throw_with_nested(std::runtime_error("failed to load pipeline"));
    }
}

void GraphicsEngine::LoadAssets()
{
    try
    {
        try
        {
            {
                D3D12_HEAP_PROPERTIES uploadHeap{};
                uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;

                D3D12_RESOURCE_DESC instanceConstantBufferDesc{};
                instanceConstantBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
                instanceConstantBufferDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
                instanceConstantBufferDesc.Width = sizeof(cbPassStruct);
                instanceConstantBufferDesc.Height = 1;
                instanceConstantBufferDesc.DepthOrArraySize = 1;
                instanceConstantBufferDesc.MipLevels = 1;
                instanceConstantBufferDesc.SampleDesc.Count = 1;
                instanceConstantBufferDesc.SampleDesc.Quality = 0;
                instanceConstantBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
                instanceConstantBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

                ThrowIfFailed(myDevice->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE,
                    &instanceConstantBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, 
                    IID_PPV_ARGS(&myInstanceConstantBuffer)));
                ThrowIfFailed(myInstanceConstantBuffer->SetName(L"myInstanceConstantBuffer"));

                D3D12_RESOURCE_DESC passConstantBufferDesc{};
                passConstantBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
                passConstantBufferDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
                passConstantBufferDesc.Width = sizeof(cbPassStruct);
                passConstantBufferDesc.Height = 1;
                passConstantBufferDesc.DepthOrArraySize = 1;
                passConstantBufferDesc.MipLevels = 1;
                passConstantBufferDesc.SampleDesc.Count = 1;
                passConstantBufferDesc.SampleDesc.Quality = 0;
                passConstantBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
                passConstantBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

                ThrowIfFailed(myDevice->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE,
                    &passConstantBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, 
                    IID_PPV_ARGS(&myPassConstantBuffer)));
                ThrowIfFailed(myPassConstantBuffer->SetName(L"myPassConstantBuffer"));
            }



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
        catch (...)
        {
            std::throw_with_nested(std::runtime_error("failed to create root signature"));
        }

        try
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
            graphicsPipelineStateDesc.VS = { reinterpret_cast<UINT8*>(defaultVsBlob->GetBufferPointer()), defaultVsBlob->GetBufferSize() };
            graphicsPipelineStateDesc.PS = { reinterpret_cast<UINT8*>(defaultPsBlob->GetBufferPointer()), defaultPsBlob->GetBufferSize() };
            graphicsPipelineStateDesc.BlendState = noBlendDesc;
            graphicsPipelineStateDesc.SampleMask = UINT_MAX;
            graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
            graphicsPipelineStateDesc.DepthStencilState.DepthEnable = FALSE;
            graphicsPipelineStateDesc.DepthStencilState.StencilEnable = FALSE;
            graphicsPipelineStateDesc.InputLayout = { inputElementDescs, sizeof(inputElementDescs) / sizeof(inputElementDescs[0]) };
            graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            graphicsPipelineStateDesc.NumRenderTargets = 1;
            graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
            graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
            graphicsPipelineStateDesc.SampleDesc.Count = 1;
            graphicsPipelineStateDesc.SampleDesc.Quality = 0;

            ThrowIfFailed(myDevice->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&myPipelineState)));
            ThrowIfFailed(myPipelineState->SetName(L"myPipelineState"));
        }
        catch (...)
        {
            std::throw_with_nested(std::runtime_error("failed to create graphics pipeline state"));
        }

        try
        {
            ThrowIfFailed(myDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                myCommandAllocator.Get(), myPipelineState.Get(), IID_PPV_ARGS(&myCommandList)));
            ThrowIfFailed(myCommandList->SetName(L"myCommandList"));
            ThrowIfFailed(myCommandList->Close());
        }
        catch (...)
        {
            std::throw_with_nested(std::runtime_error("failed to create command list"));
        }

        try
        {
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

            D3D12_HEAP_PROPERTIES heapProperties{};
            heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

            D3D12_RESOURCE_DESC vertexBufferDesc{};
            vertexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            vertexBufferDesc.Alignment = 0;
            vertexBufferDesc.Width = sizeof(vertices);
            vertexBufferDesc.Height = 1;
            vertexBufferDesc.DepthOrArraySize = 1;
            vertexBufferDesc.MipLevels = 1;
            vertexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
            vertexBufferDesc.SampleDesc.Count = 1;
            vertexBufferDesc.SampleDesc.Quality = 0;
            vertexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            vertexBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

            ThrowIfFailed(myDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE,
                &vertexBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&myVertexBuffer)));
            ThrowIfFailed(myVertexBuffer->SetName(L"myVertexBuffer"));

            D3D12_RANGE range{ 0, 0 };
            void* pVertexDataBegin{};
            ThrowIfFailed(myVertexBuffer->Map(0, &range, &pVertexDataBegin));
            std::memcpy(pVertexDataBegin, vertices, sizeof(vertices));
            myVertexBuffer->Unmap(0, nullptr);

            myVertexBufferView.BufferLocation = myVertexBuffer->GetGPUVirtualAddress();
            myVertexBufferView.StrideInBytes = sizeof(vertices[0]);
            myVertexBufferView.SizeInBytes = sizeof(vertices);
        }
        catch (...)
        {
            std::throw_with_nested(std::runtime_error("failed to create vertex buffer"));
        }
        try
        {
            const Index indices[36] = { 
                 0,  1,  2,
                 2,  3,  0,
                 4,  5,  6,
                 6,  7,  4,
                 8,  9, 10,
                10, 11,  8,
                12, 13, 14,
                14, 15, 12,
                16, 17, 18,
                18, 19, 16,
                20, 21, 22,
                22, 23, 20
            };

            D3D12_HEAP_PROPERTIES heapProperties{};
            heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

            D3D12_RESOURCE_DESC indexBufferDesc{};
            indexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            indexBufferDesc.Alignment = 0;
            indexBufferDesc.Width = sizeof(indices);
            indexBufferDesc.Height = 1;
            indexBufferDesc.DepthOrArraySize = 1;
            indexBufferDesc.MipLevels = 1;
            indexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
            indexBufferDesc.SampleDesc.Count = 1;
            indexBufferDesc.SampleDesc.Quality = 0;
            indexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            indexBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

            ThrowIfFailed(myDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE,
                &indexBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&myIndexBuffer)));
            ThrowIfFailed(myIndexBuffer->SetName(L"myIndexBuffer"));

            D3D12_RANGE range{ 0, 0 };
            void* pVertexDataBegin{};
            ThrowIfFailed(myIndexBuffer->Map(0, &range, &pVertexDataBegin));
            std::memcpy(pVertexDataBegin, indices, sizeof(indices));
            myIndexBuffer->Unmap(0, nullptr);

            myIndexBufferView.BufferLocation = myIndexBuffer->GetGPUVirtualAddress();
            myIndexBufferView.SizeInBytes = sizeof(indices);
            myIndexBufferView.Format = DXGI_FORMAT_R16_UINT;
        }
        catch (...)
        {
            std::throw_with_nested(std::runtime_error("failed to create index buffer"));
        }
        try
        {
            ThrowIfFailed(myDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&myFence)));
            ThrowIfFailed(myFence->SetName(L"myFence"));
            myFenceValue = 1;
            myFenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
            if (myFenceEvent == nullptr)
            {
                ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
            }
            WaitForPreviousFrame();
        }
        catch (...)
        {
            std::throw_with_nested(std::runtime_error("failed to create fence"));
        }
    }
    catch (...)
    {
        std::throw_with_nested(std::runtime_error("failed to load assets"));
    }
}

void GraphicsEngine::OnUpdate(float aDeltaSeconds, float aTimeSeconds, const Mat4& aToViewMatrix, const Mat4& aToProjectionMatrix)
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

    D3D12_RANGE range{ 0, 0 };
    void* pData = nullptr;
        
        
    cbInstanceStruct instance{};
    instance.transform = Mat4::TranslationMatrix(0.f, 0.f, 200);

    myInstanceConstantBuffer->Map(0, &range, &pData);
    std::memcpy(pData, &instance, sizeof(instance));
    myInstanceConstantBuffer->Unmap(0, nullptr);

    cbPassStruct pass{};
    pass.VP = aToViewMatrix * aToProjectionMatrix;
    pass.timeSeconds = aTimeSeconds;
    pass.deltaSeconds = aDeltaSeconds;

    myPassConstantBuffer->Map(0, &range, &pData);
    std::memcpy(pData, &pass, sizeof(pass));
    myPassConstantBuffer->Unmap(0, nullptr);
}

void GraphicsEngine::OnRender()
{
    try
    {
        PopulateCommandList();

        ID3D12CommandList* ppCommandLists[] = { myCommandList.Get() };
        myCommandQueue->ExecuteCommandLists(sizeof(ppCommandLists) / sizeof(ppCommandLists[0]), ppCommandLists);

        mySwapChain->Present(1, 0);
        WaitForPreviousFrame();
    }
    catch (...)
    {
        std::throw_with_nested(std::runtime_error("graphics engine failed to render"));
    }
}

void GraphicsEngine::OnDestroy()
{
    try
    {
        // Wait for the GPU to be done with all resources.
        WaitForPreviousFrame();
        CloseHandle(myFenceEvent);
    }
    catch (...)
    {
        std::throw_with_nested("graphics engine failed to destroy");
    }
}

void GraphicsEngine::PopulateCommandList()
{
    try
    {
        ThrowIfFailed(myCommandAllocator->Reset());
        ThrowIfFailed(myCommandList->Reset(myCommandAllocator.Get(), myPipelineState.Get()));

        myCommandList->SetGraphicsRootSignature(myRootSignature.Get());
        myCommandList->SetGraphicsRootConstantBufferView(0, myInstanceConstantBuffer->GetGPUVirtualAddress());
        myCommandList->SetGraphicsRootConstantBufferView(1, myPassConstantBuffer->GetGPUVirtualAddress());
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
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(myRtvHeap->GetCPUDescriptorHandleForHeapStart());
        rtvHandle.ptr += (SIZE_T)myFrameIndex * (SIZE_T)myRtvDescriptorSize;
        myCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);


        const float clearColor[] = { 0.f, 0.2f, 0.4f, 1.f };
        myCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
        myCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        myCommandList->IASetVertexBuffers(0, 1, &myVertexBufferView);
        myCommandList->IASetIndexBuffer(&myIndexBufferView);
        myCommandList->DrawIndexedInstanced(36, 1, 0, 0, 0); //TODO: maybe DrawIndexInstanced

        {
            D3D12_RESOURCE_BARRIER rtvResourceBarrier{};
            rtvResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            rtvResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            rtvResourceBarrier.Transition.pResource = myBackBuffers[myFrameIndex].Get();
            rtvResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            rtvResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            rtvResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
            myCommandList->ResourceBarrier(1, &rtvResourceBarrier);
        }
        ThrowIfFailed(myCommandList->Close());
    }
    catch (...)
    {
        std::throw_with_nested(std::runtime_error("failed to populate command list"));
    }
}

void GraphicsEngine::WaitForPreviousFrame()
{
    try
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
            WaitForSingleObject(myFenceEvent, INFINITE);
        }

        myFrameIndex = mySwapChain->GetCurrentBackBufferIndex();
    }
    catch (...)
    {
        std::throw_with_nested(std::runtime_error("failed to wait for previous frame"));
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE GraphicsEngine::DepthStencilView() const
{
    return myDsvHeap->GetCPUDescriptorHandleForHeapStart();
}