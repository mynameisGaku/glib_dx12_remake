#include "Constant.hlsli"

struct PSInput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

struct PSOutput
{
    float4 color : SV_TARGET;
};

PSOutput main(PSInput input)
{
    PSOutput output;
    output.color = float4(Texture.Sample(Sampler, input.uv)) * Diffuse;
	return output;
}