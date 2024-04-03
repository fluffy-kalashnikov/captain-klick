#include "includeConstantBuffers.hlsli.h"
#include "Model.hlsli"

PixelOut main(VertexOut input)
{
    PixelOut output;
    output.color = cbInstance.color;
    return output;
}