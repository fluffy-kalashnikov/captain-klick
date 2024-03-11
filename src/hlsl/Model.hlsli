
struct VertexIn
{
    float4 position : POSITION;
};
struct VertexOut
{
    float4 position : SV_POSITION;
};
struct PixelOut
{
    float4 color : SV_TARGET;
};