#include "Model.hlsli"

PixelOut main(VertexOut input)
{
    PixelOut output;
    output.color = input.color;
    return output;
}