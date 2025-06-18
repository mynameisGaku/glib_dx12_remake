#include "Constant.hlsli"

struct VSOutput
{
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD;
};

struct PSOutput
{
    float4 Color : SV_TARGET0;
};

PSOutput main(VSOutput input)
{
    PSOutput output = (PSOutput) 0;
    
    output.Color = ColorMap.Sample(ColorSmp, input.Texcoord);
    
    return output;
}