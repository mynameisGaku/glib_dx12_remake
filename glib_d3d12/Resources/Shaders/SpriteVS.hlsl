#include "Constant.hlsli"

struct VSInput
{
    float4 pos : POSITION;
    float2 uv : TEXCOORD;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.pos = mul(Mat, input.pos);
    output.uv = input.uv;
    
    return output;
}