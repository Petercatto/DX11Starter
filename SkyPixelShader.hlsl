#include "ShaderIncludes.hlsli"

TextureCube CubeMap : register(t0);
SamplerState BasicSampler : register(s0);

//entry point for the pixel shader
float4 main(VertexToPixel_Sky input) : SV_TARGET
{
    //sample the direction
    return CubeMap.Sample(BasicSampler, input.sampleDir);
}