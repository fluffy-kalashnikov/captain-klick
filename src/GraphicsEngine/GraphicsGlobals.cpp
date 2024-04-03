#include "GraphicsEngine/GraphicsGlobals.h"
#include "GraphicsEngine/GraphicsDevice.h"
#define WIN32_MEAN_AND_LEAN

namespace GraphicsGlobals
{
	void Create(GraphicsDevice* aDevice, const ComPtr<ID3D12GraphicsCommandList>& aCommandList)
	{
        //root signature
        D3D12_ROOT_PARAMETER rootParameters[2]{};
        {
            D3D12_ROOT_PARAMETER& instanceBuffer = rootParameters[0];
            instanceBuffer.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            instanceBuffer.Descriptor.ShaderRegister = 0;
            instanceBuffer.Descriptor.RegisterSpace = 0;
            instanceBuffer.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

            D3D12_ROOT_PARAMETER& passBuffer = rootParameters[1];
            passBuffer.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            passBuffer.Descriptor.ShaderRegister = 1;
            passBuffer.Descriptor.RegisterSpace = 0;
            passBuffer.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        }

        D3D12_STATIC_SAMPLER_DESC staticSamplers[1]{};
        {
            D3D12_STATIC_SAMPLER_DESC& trilinearWrap = staticSamplers[0];
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
        rootSignatureDesc.NumStaticSamplers = sizeof(staticSamplers) / sizeof(staticSamplers[0]);
        rootSignatureDesc.pStaticSamplers = staticSamplers;
        rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;

		globalRootSignature = aDevice->CreateRootSignature(L"globalRootSignature", rootSignatureDesc);



        //pipeline state objects
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

        D3D12_DEPTH_STENCIL_DESC noDepthStencilDesc(depthStencilDesc);
        noDepthStencilDesc.DepthEnable = FALSE;

        D3D12_INPUT_ELEMENT_DESC vertexInputLayout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        ComPtr<ID3DBlob> modelVs, modelPs, workVs, workPs, sprite3dVs, sprite3dGs, sprite3dPs;
        ThrowIfFailed(D3DReadFileToBlob(L"Model_VS.cso", &modelVs));
        ThrowIfFailed(D3DReadFileToBlob(L"Model_PS.cso", &modelPs));
        ThrowIfFailed(D3DReadFileToBlob(L"Work_VS.cso", &workVs));
        ThrowIfFailed(D3DReadFileToBlob(L"Work_PS.cso", &workPs));
        ThrowIfFailed(D3DReadFileToBlob(L"Sprite3D_VS.cso", &sprite3dVs));
        ThrowIfFailed(D3DReadFileToBlob(L"Sprite3D_GS.cso", &sprite3dGs));
        ThrowIfFailed(D3DReadFileToBlob(L"Sprite3D_PS.cso", &sprite3dPs));

        D3D12_GRAPHICS_PIPELINE_STATE_DESC modelPipelineStateDesc{};
        modelPipelineStateDesc.pRootSignature = globalRootSignature.Get();
        modelPipelineStateDesc.InputLayout = { vertexInputLayout, sizeof(vertexInputLayout) / sizeof(vertexInputLayout[0]) };
        modelPipelineStateDesc.VS = CD3DX12_SHADER_BYTECODE(modelVs->GetBufferPointer(), modelVs->GetBufferSize());
        modelPipelineStateDesc.PS = CD3DX12_SHADER_BYTECODE(modelPs->GetBufferPointer(), modelPs->GetBufferSize());
        modelPipelineStateDesc.BlendState = noBlendDesc;
        modelPipelineStateDesc.SampleMask = UINT_MAX;
        modelPipelineStateDesc.RasterizerState = rasterizerDesc;
        modelPipelineStateDesc.DepthStencilState = depthStencilDesc;
        modelPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        modelPipelineStateDesc.NumRenderTargets = 1;
        modelPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        modelPipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        modelPipelineStateDesc.SampleDesc.Count = 1;
        modelPipelineStateDesc.SampleDesc.Quality = 0;

        D3D12_GRAPHICS_PIPELINE_STATE_DESC workPipelineStateDesc{};
        workPipelineStateDesc.pRootSignature = globalRootSignature.Get();
        workPipelineStateDesc.VS = CD3DX12_SHADER_BYTECODE(workVs->GetBufferPointer(), workVs->GetBufferSize());
        workPipelineStateDesc.PS = CD3DX12_SHADER_BYTECODE(workPs->GetBufferPointer(), workPs->GetBufferSize());
        workPipelineStateDesc.BlendState = noBlendDesc;
        workPipelineStateDesc.SampleMask = UINT_MAX;
        workPipelineStateDesc.RasterizerState = rasterizerDesc;
        workPipelineStateDesc.DepthStencilState = noDepthStencilDesc;
        workPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        workPipelineStateDesc.NumRenderTargets = 1;
        workPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        workPipelineStateDesc.SampleDesc.Count = 1;
        workPipelineStateDesc.SampleDesc.Quality = 0;

        globalModelPipeline = aDevice->CreatePipelineState(L"globalModelPipeline", modelPipelineStateDesc);
		globalWorkPipeline = aDevice->CreatePipelineState(L"globalWorkPipeline", workPipelineStateDesc);





        //mesh primitives
        const Vertex cubeVertices[24] = {
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
        const Index cubeIndices[36] = {
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
        vertexBuffer = aDevice->CreateDefaultBuffer(L"cube vertex buffer", aCommandList.Get(), cubeVertices);
        indexBuffer = aDevice->CreateDefaultBuffer(L"cube index buffer", aCommandList.Get(), cubeIndices);
        globalCubeMesh.Init(L"cube", std::move(vertexBuffer), std::move(indexBuffer));


        const Vertex planeVertices[4] = {
            { -50,-50, 0 },
            { -50, 50, 0 },
            {  50, 50, 0 },
            {  50,-50, 0 },
        };
        const Index planeIndices[6] = {
            0, 1, 2,
            0, 2, 3,
        };
        vertexBuffer = aDevice->CreateDefaultBuffer(L"plane vertex buffer", aCommandList.Get(), planeVertices);
        indexBuffer = aDevice->CreateDefaultBuffer(L"plane index buffer", aCommandList.Get(), planeIndices);
        globalPlaneMesh.Init(L"plane", std::move(vertexBuffer), std::move(indexBuffer));
    }

	void Destroy()
	{
        globalCubeMesh = {};
        globalPlaneMesh = {};
		globalWorkPipeline = nullptr;
        globalModelPipeline = nullptr;
		globalRootSignature = nullptr;
	}
}
