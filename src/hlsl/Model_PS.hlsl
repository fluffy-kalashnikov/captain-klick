#include "ConstantBuffers.hlsli.h"
#include "Model.hlsli"

PixelOut main(VertexOut input)
{
    PixelOut output;
    output.color = input.color * (sin(cbPass.timeSeconds) * 0.5 + 0.5);
    return output;
}