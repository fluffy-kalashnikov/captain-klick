#include "includeConstantBuffers.hlsli.h"
#include "Model.hlsli"

VertexOut main(VertexIn input)
{
    float4 position = input.position;
    position = mul(cbInstance.transform, position);
    position = mul(cbPass.cameraVP, position);
    
    VertexOut output;
    output.position = position;
	return output; 
}