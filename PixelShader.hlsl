#include "ShaderIncludes.hlsli"

//constant buffer definition
cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float roughness;
    float3 cameraPosition;
    float3 ambient;
    Light lights[MAX_LIGHTS];
}

//calculate the diffuse
float3 Diffuse(Light light, float3 normal, float3 worldPos)
{
    if (light.Type == 0)
    {
        return (saturate(dot(normal, -light.Direction))); //normalized direction to the light
    }
    else
    {
        return (saturate(dot(normal, normalize(light.Position - worldPos)))); //normalized direction to the light
    }
}

//calculate the specular
float Specular(Light light, float3 normal, float3 worldPos)
{
    float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
    float3 V = normalize(cameraPosition - worldPos);
    float3 R;
    if (light.Type == 0)
    {
        R = reflect(normalize(light.Direction), normal);
    }
    else
    {
        R = reflect(-normalize(light.Position - worldPos), normal);
    }
    float spec;
    if(specExponent > 0.05f)
    {
        spec = pow(saturate(dot(R, V)), specExponent);
    }
    else
    {
        spec = 0.0f;
    }
    return spec;
}

//calculate the attenuation
float Attenuate(Light light, float3 worldPos)
{
    float dist = distance(light.Position, worldPos);
    float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
    return att * att;
}

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
    
    //normalize normals
    input.normal = normalize(input.normal);
    
    float3 finalColor = (0.0f, 0.0f, 0.0f);
    
    //add diffuse and specular for each light
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].Type == 0)
        {
            finalColor += (Diffuse(lights[i], input.normal, input.worldPosition) + Specular(lights[i], input.normal, input.worldPosition)) * lights[i].Color * lights[i].Intensity * colorTint.xyz;
        }
        else
        {
            finalColor += (Diffuse(lights[i], input.normal, input.worldPosition) + Specular(lights[i], input.normal, input.worldPosition)) * lights[i].Color * Attenuate(lights[i], input.worldPosition) * colorTint.xyz;
        }
    }
    
    return (float4(ambient, 1.0f) * colorTint) + float4(finalColor, 1.0f);
}