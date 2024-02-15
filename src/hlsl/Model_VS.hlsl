#include "Model.hlsli"

VertexOut main(VertexIn input)
{
    VertexOut output;
    output.position = input.position;
    output.color = input.color;
	return output;
}