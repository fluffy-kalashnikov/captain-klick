#include "pch.h"
#include "cbFrame.h"
#include "GraphicsEngine.h"
#include "Vertex.h"

void GraphicsEngine::OnInitialize(HWND aHwnd)
{
    try
    {
        myHwnd = aHwnd;
        OnUpdate();
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

        {
            D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
            commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            ThrowIfFailed(myDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&myCommandQueue)));
        }
        {
            DXGI_SWAP_CHAIN_DESC swapChainDesc{};
            swapChainDesc.BufferCount = FRAME_COUNT;
            swapChainDesc.BufferDesc.Width = myWidth;
            swapChainDesc.BufferDesc.Height = myHeight;
            swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            swapChainDesc.OutputWindow = myHwnd;
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.Windowed = TRUE;

            ComPtr<IDXGISwapChain> swapChain;
            ThrowIfFailed(myFactory->CreateSwapChain(myCommandQueue.Get(), &swapChainDesc, &swapChain));
            ThrowIfFailed(swapChain.As(&mySwapChain));
        }
        ThrowIfFailed(myFactory->MakeWindowAssociation(myHwnd, DXGI_MWA_NO_ALT_ENTER));
        myFrameIndex = mySwapChain->GetCurrentBackBufferIndex();
        {
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
            rtvHeapDesc.NumDescriptors = FRAME_COUNT;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            ThrowIfFailed(myDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&myRtvHeap)));
            myRtvHeap->SetName(L"myRtvHeap");
            myRtvDescriptorSize = myDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        }
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = myRtvHeap->GetCPUDescriptorHandleForHeapStart();
            for (UINT n = 0; n < FRAME_COUNT; n++)
            {
                ThrowIfFailed(mySwapChain->GetBuffer(n, IID_PPV_ARGS(&myRenderTargets[n])));
                myDevice->CreateRenderTargetView(myRenderTargets[n].Get(), nullptr, rtvHandle);
                rtvHandle.ptr += myRtvDescriptorSize;

                myRenderTargets[n]->SetName(L"myRenderTagrgets[x]");
            }
        }
        {
            D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavHeapDesc{};
            cbvSrvUavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            cbvSrvUavHeapDesc.NumDescriptors = 512; //TODO: figure out actual number of shite required
            cbvSrvUavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            ThrowIfFailed(myDevice->CreateDescriptorHeap(&cbvSrvUavHeapDesc, IID_PPV_ARGS(&myCbvSrvHeap)));
            myCbvSrvHeap->SetName(L"myCbvSrvUavHeap");
            myCbvSrvDescriptorSize = myDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }

        ThrowIfFailed(myDevice->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&myCommandAllocator)));
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
                cbFrame frame;
                frame.deltaSeconds = 69;
                frame.timeSeconds = 420;
                frame.transform = Mat4::Identity;


                D3D12_HEAP_PROPERTIES heapProperties{};
                heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

                D3D12_RESOURCE_DESC resourceDesc{};
                resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
                resourceDesc.Width = sizeof(frame);
                resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

                ThrowIfFailed(myDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE,
                    &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&myFrameConstantBuffer)));


                D3D12_CPU_DESCRIPTOR_HANDLE handle = myCbvSrvHeap->GetCPUDescriptorHandleForHeapStart();
                
                //TODO: make actual constant buffer instance on GPU
                D3D12_CONSTANT_BUFFER_VIEW_DESC frameBufferViewDesc{};
                frameBufferViewDesc.BufferLocation = myFrameConstantBuffer->GetGPUVirtualAddress();
                frameBufferViewDesc.SizeInBytes = sizeof(frame);
                myDevice->CreateConstantBufferView(&frameBufferViewDesc, handle);
            }



            D3D12_ROOT_PARAMETER rootParameters[1]{};
            {
                D3D12_ROOT_PARAMETER& frameBuffer = rootParameters[0];
                frameBuffer.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
                frameBuffer.Descriptor.ShaderRegister = 0; //TODO: set to actual constant buffer descriptor
                frameBuffer.Descriptor.RegisterSpace = 0;
                frameBuffer.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
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

            D3D12_BLEND_DESC blendDesc{};
            blendDesc.AlphaToCoverageEnable = FALSE;
            blendDesc.IndependentBlendEnable = FALSE;
            blendDesc.RenderTarget[0].BlendEnable = FALSE;
            blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
            blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
            blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
            blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
            blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
            blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
            blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

            //D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
            //depthStencilDesc.DepthEnable = TRUE; //usually true, but not for the triangle demo
            //depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            //depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
            //depthStencilDesc.StencilEnable = FALSE;
            //depthStencilDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
            //depthStencilDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
            //depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
            //depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
            //depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
            //depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            //depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
            //depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
            //depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
            //depthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;


            D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
            graphicsPipelineStateDesc.pRootSignature = myRootSignature.Get();
            graphicsPipelineStateDesc.VS = { reinterpret_cast<UINT8*>(defaultVsBlob->GetBufferPointer()), defaultVsBlob->GetBufferSize() };
            graphicsPipelineStateDesc.PS = { reinterpret_cast<UINT8*>(defaultPsBlob->GetBufferPointer()), defaultPsBlob->GetBufferSize() };
            //graphicsPipelineStateDesc.DS;
            //graphicsPipelineStateDesc.HS;
            //graphicsPipelineStateDesc.GS;
            //graphicsPipelineStateDesc.StreamOutput;
            graphicsPipelineStateDesc.BlendState = blendDesc;
            graphicsPipelineStateDesc.SampleMask = UINT_MAX;
            graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
            graphicsPipelineStateDesc.DepthStencilState.DepthEnable = FALSE;
            graphicsPipelineStateDesc.DepthStencilState.StencilEnable = FALSE;
            graphicsPipelineStateDesc.InputLayout = { inputElementDescs, sizeof(inputElementDescs) / sizeof(inputElementDescs[0]) };
            //graphicsPipelineStateDesc.IBStripCutValue;
            graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            graphicsPipelineStateDesc.NumRenderTargets = 1;
            graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
            //graphicsPipelineStateDesc.DSVFormat;
            graphicsPipelineStateDesc.SampleDesc.Count = 1;
            //graphicsPipelineStateDesc.SampleDesc.Quality;
            //graphicsPipelineStateDesc.NodeMask;
            //graphicsPipelineStateDesc.CachedPSO;
            //graphicsPipelineStateDesc.Flags;
            myDevice->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&myPipelineState));
        }
        catch (...)
        {
            std::throw_with_nested(std::runtime_error("failed to create graphics pipeline state"));
        }

        try
        {
            ThrowIfFailed(myDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                myCommandAllocator.Get(), myPipelineState.Get(), IID_PPV_ARGS(&myCommandList)));
            ThrowIfFailed(myCommandList->Close());
        }
        catch (...)
        {
            std::throw_with_nested(std::runtime_error("failed to create command list"));
        }

        try
        {
            const Vec4 color = { 1.f, 1.f, 0.f, 1.f };
            const Vertex vertices[3] = {
                { { 0.0f, 0.25f * myAspectRatio, 0.0f, 1.f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
                { { 0.25f, -0.25f * myAspectRatio, 0.0f, 1.f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
                { { -0.25f, -0.25f * myAspectRatio, 0.0f, 1.f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
            };

            D3D12_HEAP_PROPERTIES heapProperties{};
            heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

            D3D12_RESOURCE_DESC resourceDesc{};
            resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            resourceDesc.Alignment = 0; //D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
            resourceDesc.Width = sizeof(vertices);
            resourceDesc.Height = 1;
            resourceDesc.DepthOrArraySize = 1;
            resourceDesc.MipLevels = 1;
            resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
            resourceDesc.SampleDesc.Count = 1;
            resourceDesc.SampleDesc.Quality = 0;
            resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

            ThrowIfFailed(myDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE,
                &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&myVertexBuffer)));

            D3D12_RANGE range{};
            range.Begin = 0;
            range.End = 0;
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
            myDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&myFence));
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

void GraphicsEngine::OnUpdate()
{
    try
    {
        GetClientRect(myHwnd, &myScissorRect);
        myWidth = myScissorRect.right;
        myHeight = myScissorRect.bottom;
        myAspectRatio = myWidth / static_cast<FLOAT>(myHeight);

        myViewport.Width = static_cast<FLOAT>(myWidth);
        myViewport.Height = static_cast<FLOAT>(myHeight);
        myViewport.MinDepth = 0;
        myViewport.MaxDepth = 1;
    }
    catch (...)
    {
        std::throw_with_nested(std::runtime_error("graphics engine failed to update"));
    }
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
        myCommandList->RSSetViewports(1, &myViewport);
        myCommandList->RSSetScissorRects(1, &myScissorRect);


        {
            D3D12_RESOURCE_BARRIER rtvResourceBarrier{};
            rtvResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            rtvResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            rtvResourceBarrier.Transition.pResource = myRenderTargets[myFrameIndex].Get();
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
        myCommandList->DrawInstanced(3, 1, 0, 0); //TODO: maybe DrawIndexInstanced

        {
            D3D12_RESOURCE_BARRIER rtvResourceBarrier{};
            rtvResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            rtvResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            rtvResourceBarrier.Transition.pResource = myRenderTargets[myFrameIndex].Get();
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