#include "Constant.hlsli"

float4 main( float4 pos : POSITION ) : SV_POSITION
{
    return mul(Mat, pos);
}