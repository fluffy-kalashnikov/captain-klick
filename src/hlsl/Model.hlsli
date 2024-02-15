
struct VertexIn
{
    float4 position : POSITION;
    float4 color    : COLOR;
};
struct VertexOut
{
    float4 position : SV_POSITION;
    float4 color    : COLOR;
};
struct PixelOut
{
    float4 color : SV_TARGET;
};