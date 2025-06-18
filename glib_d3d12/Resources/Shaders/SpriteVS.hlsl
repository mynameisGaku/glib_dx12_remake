#include "Constant.hlsli"

struct VSInput
{
    float3 Position : POSITION;
    float2 Texcoord : TEXCOORD;
};

struct VSOutput
{
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    
    float4 localPos = float4(input.Position, 1.0f);
    float4 worldPos = mul(World, localPos);
    float4 viewPos = mul(View, worldPos);
    float4 projPos = mul(Proj, viewPos);
    
    output.Position = projPos;
    output.Texcoord = input.Texcoord;

    return output;
}