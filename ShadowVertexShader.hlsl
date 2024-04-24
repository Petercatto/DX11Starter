#include "ShaderIncludes.hlsli"

//constant buffer for external data
cbuffer externalData : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
};

//vertex shader for shadow maps
float4 main(VertexShaderInput input) : SV_POSITION
{
    matrix wvp = mul(projection, mul(view, world));
    return mul(wvp, float4(input.localPosition, 1.0f));
}