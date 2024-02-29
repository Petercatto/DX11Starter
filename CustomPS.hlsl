
struct VertexToPixel
{
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD;
};

//constant buffer definition
cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float totalTime;
}

float4 main(VertexToPixel input) : SV_TARGET
{
    //uv and color
    float2 uv = input.uv;
    float3 color = float3(0, 0, 0);
    
    //color gradient that changes over time
    float red = sin(uv.x * 10 + totalTime) * 0.5 + 0.5;
    float green = cos(uv.y * 10 + totalTime) * 0.5 + 0.5;
    float blue = sin((uv.x + uv.y) * 10 + totalTime) * 0.5 + 0.5;
    float3 gradient = float3(red, green, blue);

    //how many tiles
    uv *= 15;
    
    //calculate hexagons
    float2 size = float2(1, 1.73);                              //size overall
    float2 center = size * 0.5;                                 //center of each
    float2 hexU = uv % size - center;                           //one row unshifted
    float2 hexV = (uv - center) % size - center;                //shifted row
    float2 prop = (length(hexU) < length(hexV)) ? hexU : hexV;  //which row is it and how should it propgate
    float2 hexID = (uv - prop) + 15;                            //id of each hexagon
    prop = abs(prop);                                           //make sure it's always propogating outwards (positive)
    float angle = dot(prop, normalize(size));                   //angle of propogation
    float propS = 0.5 - max(angle, prop.x);                     //propogation speed
    float3 hexagon = float3(propS, hexID.x, hexID.y);           //make the actual hexagons
    
    //animation
    color += smoothstep(0.05, 0.1, hexagon.x * sin(hexagon.z * hexagon.y + totalTime));
    //change the color
    color *= gradient;

    return float4(color, 1.0);
}