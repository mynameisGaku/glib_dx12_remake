cbuffer Transform : register(b0)
{
    float4x4 World : packoffset(c0);
    float4x4 View : packoffset(c4);
    float4x4 Proj : packoffset(c8);
};

Texture2D<float4> Texture : register(t0);
SamplerState Sampler : register(s0);