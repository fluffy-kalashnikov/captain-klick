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
    graphicsPipelineStateDesc.pRootSignature = myGraphicsDevice.RootSignature();
    graphicsPipelineStateDesc.BlendState = noBlendDesc;
    graphicsPipelineStateDesc.SampleMask = UINT_MAX;
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
    graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleDesc.Quality = 0;


    {
        ComPtr<ID3DBlob> workVs, workPs;
        ThrowIfFailed(D3DReadFileToBlob(L"Work_VS.cso", &workVs));
        ThrowIfFailed(D3DReadFileToBlob(L"Work_PS.cso", &workPs));

        graphicsPipelineStateDesc.DepthStencilState.DepthEnable = FALSE;
        graphicsPipelineStateDesc.VS = CD3DX12_SHADER_BYTECODE(workVs->GetBufferPointer(), workVs->GetBufferSize());
        graphicsPipelineStateDesc.PS = CD3DX12_SHADER_BYTECODE(workPs->GetBufferPointer(), workPs->GetBufferSize());
        myWorkPipelineState = myGraphicsDevice.CreateGraphicsPipelineState(L"myWorkPipelineState", graphicsPipelineStateDesc);
    }
    {
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        ComPtr<ID3DBlob> modelVs, modelPs;
        ThrowIfFailed(D3DReadFileToBlob(L"Model_VS.cso", &modelVs));
        ThrowIfFailed(D3DReadFileToBlob(L"Model_PS.cso", &modelPs));

        graphicsPipelineStateDesc.InputLayout = { inputElementDescs, sizeof(inputElementDescs) / sizeof(inputElementDescs[0]) };
        graphicsPipelineStateDesc.VS = CD3DX12_SHADER_BYTECODE(modelVs->GetBufferPointer(), modelVs->GetBufferSize());
        graphicsPipelineStateDesc.PS = CD3DX12_SHADER_BYTECODE(modelPs->GetBufferPointer(), modelPs->GetBufferSize());
        graphicsPipelineStateDesc.DepthStencilState.DepthEnable = TRUE;
        myCubePipelineState = myGraphicsDevice.CreateGraphicsPipelineState(L"myCubePipelineState", graphicsPipelineStateDesc);
    }

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
    
    commandList->SetPipelineState(myWorkPipelineState.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    for (int n = 0; n < 2000; n++)
    {
        commandList->DrawInstanced(4, 1, 0, 0);
    }

    commandList->SetPipelineState(myCubePipelineState.Get());
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