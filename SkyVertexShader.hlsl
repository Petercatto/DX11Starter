#include "ShaderIncludes.hlsli"

//constant buffer definition
cbuffer ExternalData : register(b0)
{
    float4x4 view;
    float4x4 projection;
}

//entry point for the vertex shader
VertexToPixel_Sky main(VertexShaderInput input)
{
    //define output
    VertexToPixel_Sky output;
    
    //zero the translation of the view matrix
    float4x4 zeroedTransView = view;
    zeroedTransView._14 = 0;
    zeroedTransView._24 = 0;
    zeroedTransView._34 = 0;
    
    //update the position
    matrix pos = mul(projection, zeroedTransView);
    output.position = mul(pos, float4(input.localPosition, 1.0f));
    
    //ensure the depth is 1
    output.position.z = output.position.w;
    
    //use the direction from the local position
    output.sampleDir = input.localPosition;
    
    return output;
}