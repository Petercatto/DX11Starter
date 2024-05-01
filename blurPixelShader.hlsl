Texture2D Pixels : register(t0);
SamplerState ClampSampler : register(s0);

//external data
cbuffer externalData : register(b0)
{
    int blurRadius;
    float pixelWidth;
    float pixelHeight;
}

struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(VertexToPixel input) : SV_TARGET
{
    //total color and samples
    float4 total = 0;
    int sampleCount = 0;
    
    //loop through box
    for (int x = -blurRadius; x <= blurRadius; x++)
    {
        for (int y = -blurRadius; y <= blurRadius; y++)
        {
            //calculate uv for sample
            float2 uv = input.uv;
            uv += float2(x * pixelWidth, y * pixelHeight);
            
            //add to color for sample
            total += Pixels.Sample(ClampSampler, uv);
            sampleCount++;
        }
    }
    
    //return average
    return total / sampleCount;
}