Texture2D Pixels : register(t0);
SamplerState ClampSampler : register(s0);

//external data
cbuffer externalData : register(b0)
{
    float3 colorOffset;
    float2 screenCenter;
}

struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(VertexToPixel input) : SV_TARGET
{
    //sample textures for red, green, and blue channels
    float colR = Pixels.Sample(ClampSampler, input.uv - float2(colorOffset.x, colorOffset.y) * 0.005).r;
    float colG = Pixels.Sample(ClampSampler, input.uv).g;
    float colB = Pixels.Sample(ClampSampler, input.uv + float2(colorOffset.y, colorOffset.z) * 0.005).b;

    //return the color
    return float4(colR, colG, colB, 1);
}