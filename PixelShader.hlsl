#include "ShaderIncludes.hlsli"

Texture2D Albedo : register(t0);          // "t" registers for textures
Texture2D NormalMap : register(t1);
Texture2D RoughnessMap : register(t2);
Texture2D MetalnessMap : register(t3);
Texture2D ShadowMap : register(t4);
SamplerState BasicSampler : register(s0); // "s" registers for samplers
SamplerComparisonState ShadowSampler : register(s1);

//constant buffer definition
cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float3 cameraPosition;
    Light lights[MAX_LIGHTS];
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
	//perform the perspective divide
    input.shadowMapPos /= input.shadowMapPos.w;
    //convert the normalized device coordinates to uv
    float2 shadowUV = input.shadowMapPos.xy * 0.5f + 0.5f;
    shadowUV.y = 1 - shadowUV.y; // Flip the Y
    //grab the distances
    float distToLight = input.shadowMapPos.z;
    float shadowAmount = ShadowMap.SampleCmpLevelZero(
        ShadowSampler,
        shadowUV,
        distToLight).r;
    
    //unpack normal map
    float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1;
    unpackedNormal = normalize(unpackedNormal);
    
    //create TBN Matrix
    float3 N = normalize(input.normal);
    float3 T = normalize(input.tangent);
    T = normalize(T - N * dot(T, N)); // Gram-Schmidt orthonormalize process
    float3 B = cross(T, N); //bitangent
    float3x3 TBN = float3x3(T, B, N);
    
    //transform normals
    input.normal = mul(unpackedNormal, TBN);
    
    //normalize normals
    input.normal = normalize(input.normal);
    
    //sample the texture (gamma corrected)
    float3 albedoColor = pow(Albedo.Sample(BasicSampler, input.uv).rgb, 2.2f);
    
    //sample roughness map
    float roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;
    
    //sample metalness map
    float metalness = MetalnessMap.Sample(BasicSampler, input.uv).r;

    // Specular color determination -----------------
    // Assume albedo texture is actually holding specular color where metalness == 1
    // Note the use of lerp here - metal is generally 0 or 1, but might be in between
    // because of linear texture sampling, so we lerp the specular color to match
    float3 specularColor = lerp(F0_NON_METAL, albedoColor.rgb, metalness);
    
    //for all lights
    float3 toCam = normalize(cameraPosition - input.worldPosition);
    float3 F;
    
    float3 finalColor = 0.0f;
    
    //add diffuse and specular for each light
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        //directional light
        if (lights[i].Type == 0)
        {
            float3 toLight = normalize(-lights[i].Direction);
            
            // Calculate the light amounts
            float diff = DiffusePBR(input.normal, toLight);
            float3 spec = MicrofacetBRDF(input.normal, toLight, toCam, roughness, specularColor, F);

            // Calculate diffuse with energy conservation, including cutting diffuse for metals
            float3 balancedDiff = DiffuseEnergyConserve(diff, F, metalness);
            
            finalColor += (balancedDiff * albedoColor + spec) * lights[i].Color * lights[i].Intensity * colorTint.xyz;
        }
        //point light
        else
        {
            float3 toLight = normalize(lights[i].Position - input.worldPosition);
            
            // Calculate the light amounts
            float diff = DiffusePBR(input.normal, toLight);
            float3 spec = MicrofacetBRDF(input.normal, toLight, toCam, roughness, specularColor, F);

            // Calculate diffuse with energy conservation, including cutting diffuse for metals
            float3 balancedDiff = DiffuseEnergyConserve(diff, F, metalness);
            
            finalColor += (balancedDiff * albedoColor + spec) * lights[i].Color * Attenuate(lights[i], input.worldPosition) * colorTint.xyz;
        }
        if (i == 0)
        {
            finalColor *= shadowAmount;
        }
    }
    
    float4 finalOutput = float4(finalColor, 1.0f);
    //gamma corrected
    return pow(finalOutput, 1.0f / 2.2f);
}