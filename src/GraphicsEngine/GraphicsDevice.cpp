#include "GraphicsEngine/GraphicsDevice.h"

void GraphicsDevice::Initialize()
{
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&myDebug)));
    myDebug->EnableDebugLayer();

    ThrowIfFailed(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&myFactory)));
    ThrowIfFailed(myFactory->EnumAdapters(0, &myAdapter));
    ThrowIfFailed(D3D12CreateDevice(myAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&myDevice)));
    myDevice->SetName(L"GraphicsDevice::myDevice");
    myCbvSrvDescriptorSize = myDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    myDsvDescriptorSize = myDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    myRtvDescriptorSize = myDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    ThrowIfFailed(myDevice->QueryInterface(IID_PPV_ARGS(&myInfoQueue)));
    ThrowIfFailed(myInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true));
    ThrowIfFailed(myInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true));
    ThrowIfFailed(myInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true));
}

ComPtr<ID3D12Resource> GraphicsDevice::CreateTexture2D(const std::wstring& aName, UINT aWidth, UINT aHeight, 
    DXGI_FORMAT aFormat, D3D12_RESOURCE_STATES aResourceStates, D3D12_RESOURCE_FLAGS aResourceFlags)
{
    try
    {
        CD3DX12_HEAP_PROPERTIES defaultHeap(D3D12_HEAP_TYPE_DEFAULT);

        D3D12_RESOURCE_DESC textureDesc{};
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        textureDesc.Alignment = 0;
        textureDesc.Width = static_cast<UINT64>(aWidth);
        textureDesc.Height = aHeight;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.MipLevels = 1;
        textureDesc.Format = aFormat;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        textureDesc.Flags = aResourceFlags;

        D3D12_CLEAR_VALUE clearValue{};
        clearValue.Format = aFormat;
        switch (aFormat)
        {
        case DXGI_FORMAT_D32_FLOAT:
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        {
            clearValue.DepthStencil.Depth = 1.f;
            clearValue.DepthStencil.Stencil = 0;
            break;
        }
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        {
            clearValue.Color[0] = 0;
            clearValue.Color[1] = 0;
            clearValue.Color[2] = 0;
            clearValue.Color[3] = 0;
            break;
        }
        default:
        {
            throw std::runtime_error("DXGI_FORMAT unknown");
            break;
        }
        }

        ComPtr<ID3D12Resource> texture2d;
        ThrowIfFailed(myDevice->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE,
            &textureDesc, aResourceStates, &clearValue, IID_PPV_ARGS(&texture2d)));
        ThrowIfFailed(texture2d->SetName(aName.c_str()));
        return texture2d;
    }
    catch (...)
    {
        std::throw_with_nested(std::runtime_error(std::format("failed to create texture 2d")));
    }
}

Buffer GraphicsDevice::CreateDefaultBuffer(const std::wstring& aName, 
    const ComPtr<ID3D12GraphicsCommandList>& aCommandList,
    const void* aBuffer, const UINT aBufferByteSize, const UINT aBufferByteStride)
{
    try
    {
        if (aCommandList == nullptr)
        {
            throw std::runtime_error("command list is null");
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
            
            aCommandList->ResourceBarrier(1, &commonCopyDest);
            aCommandList->CopyResource(blobGPU.Get(), blobUpload.Get());
            aCommandList->ResourceBarrier(1, &copyDestGenericRead);
        }

        Buffer buffer;
        buffer.myBlobCPU = std::move(blobCPU);
        buffer.myBlobGPU = std::move(blobGPU);
        buffer.myBlobUpload = std::move(blobUpload);
        buffer.myBlobByteSize = aBufferByteSize;
        buffer.myBlobByteStride = aBufferByteStride;
        return buffer;
    }
    catch (...)
    {
        std::throw_with_nested("failed to create default buffer");
    }
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

ComPtr<ID3D12RootSignature> GraphicsDevice::CreateRootSignature(const std::wstring& aName,
    const D3D12_ROOT_SIGNATURE_DESC& aRootSignatureDesc)
{
    ComPtr<ID3DBlob> rootSignatureBlob, rootSignatureError;
    ThrowIfFailed(D3D12SerializeRootSignature(&aRootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSignatureBlob, &rootSignatureError));

    ComPtr<ID3D12RootSignature> rootSignature;
    ThrowIfFailed(myDevice->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
        rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
    ThrowIfFailed(rootSignature->SetName(aName.c_str()));
    return rootSignature;
}

ComPtr<ID3D12PipelineState> GraphicsDevice::CreatePipelineState(const std::wstring& aName,
    const D3D12_GRAPHICS_PIPELINE_STATE_DESC& aPipelineStateDesc)
{
    ComPtr<ID3D12PipelineState> pipelineState;
    ThrowIfFailed(myDevice->CreateGraphicsPipelineState(&aPipelineStateDesc, IID_PPV_ARGS(&pipelineState)));
    ThrowIfFailed(pipelineState->SetName(aName.c_str()));
    return pipelineState;
}

ComPtr<ID3D12DescriptorHeap> GraphicsDevice::CreateDescriptorHeap(const std::wstring& aName, const D3D12_DESCRIPTOR_HEAP_DESC& aDescriptorHeapDesc)
{
    ComPtr<ID3D12DescriptorHeap> descriptorHeap;
    ThrowIfFailed(myDevice->CreateDescriptorHeap(&aDescriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap)));
    ThrowIfFailed(descriptorHeap->SetName(aName.c_str()));
    return descriptorHeap;
}

ComPtr<ID3D12Fence> GraphicsDevice::CreateFence(const std::wstring& aName)
{
    ComPtr<ID3D12Fence> fence;
    ThrowIfFailed(myDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
    ThrowIfFailed(fence->SetName(aName.c_str()));
    return fence;
}

ComPtr<ID3D12CommandAllocator> GraphicsDevice::CreateCommandAllocator(const std::wstring& aName)
{
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ThrowIfFailed(myDevice->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
    ThrowIfFailed(commandAllocator->SetName(aName.c_str()));
    return commandAllocator;
}

ComPtr<ID3D12GraphicsCommandList> GraphicsDevice::CreateGraphicsCommandList(const std::wstring& aName, const ComPtr<ID3D12PipelineState>& aPipelineState, const ComPtr<ID3D12CommandAllocator>& aCommandAllocator)
{
    ComPtr<ID3D12GraphicsCommandList> graphicsCommandList;
    ThrowIfFailed(myDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
        aCommandAllocator.Get(), aPipelineState.Get(), IID_PPV_ARGS(&graphicsCommandList)));
    ThrowIfFailed(graphicsCommandList->SetName(aName.c_str()));
    return graphicsCommandList;
}

ComPtr<ID3D12CommandQueue> GraphicsDevice::CreateDirectCommandQueue(const std::wstring& aName)
{
    ComPtr<ID3D12CommandQueue> commandQueue;
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
    commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ThrowIfFailed(myDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue)));
    ThrowIfFailed(commandQueue->SetName(aName.c_str()));
    return commandQueue;
}

void GraphicsDevice::CreateRenderTargetView(const ComPtr<ID3D12Resource>& aResource, D3D12_CPU_DESCRIPTOR_HANDLE aRtvHeapHandle)
{
    myDevice->CreateRenderTargetView(aResource.Get(), nullptr, aRtvHeapHandle);
}

void GraphicsDevice::CreateDepthStencilView(const ComPtr<ID3D12Resource>& aResource, D3D12_CPU_DESCRIPTOR_HANDLE aDsvHeapHandle)
{
    myDevice->CreateDepthStencilView(aResource.Get(), nullptr, aDsvHeapHandle);
}

ComPtr<IDXGISwapChain3> GraphicsDevice::CreateSwapChain(const ComPtr<ID3D12CommandQueue>& aCommandQueue, HWND aHwnd, UINT aBufferCount)
{
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
    swapChainDesc.Width = 0;
    swapChainDesc.Height = 0;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = aBufferCount;
    swapChainDesc.Scaling = DXGI_SCALING_NONE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = 0;

    ComPtr<IDXGISwapChain1> swapChain1;
    ThrowIfFailed(myFactory->CreateSwapChainForHwnd(aCommandQueue.Get(), aHwnd, &swapChainDesc, nullptr, nullptr, &swapChain1));
    ThrowIfFailed(myFactory->MakeWindowAssociation(aHwnd, DXGI_MWA_NO_ALT_ENTER));

    ComPtr<IDXGISwapChain3> swapChain3;
    ThrowIfFailed(swapChain1.As(&swapChain3));
    return swapChain3;
}