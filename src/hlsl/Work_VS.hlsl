#include "Work.hlsli"

VertexOut main(const unsigned int aVertexID : SV_VERTEXID)
{
    const float4 position[4] =
    {
        float4(-1, -1, 0, 1),
        float4(-1, 1, 0, 1),
        float4(1, -1, 0, 1),
        float4(1, 1, 0, 1)
    };
    const float2 uv[4] =
    {
        float2(0, 1),
        float2(0, 0),
        float2(1, 1),
        float2(1, 0)
    };
    VertexOut output;
    output.Position = position[aVertexID];
    output.UV = uv[aVertexID];
    return output;
}