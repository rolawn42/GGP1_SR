#include "ShaderIncludes.hlsli"

cbuffer psConstantBuffer : register(b0)
{
    float4 colorTint;
    float2 uvScale;
    float2 uvOffset;
    float3 cameraPosition;
    float roughness;
    
    float3 ambient;
    Light lights[MAX_LIGHTS];
    int lightCount;
    
}

Texture2D SurfaceTexture : register(t0); // "t" registers for textures
Texture2D NormalMap : register(t1); // "t" registers for textures
SamplerState BasicSampler : register(s0); // "s" registers for samplers


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
    //apply UV alterations
    input.uv = input.uv * uvScale + uvOffset;
    
    //normalize the input normals and tangent
    input.normal = normalize(input.normal);
    input.tangent = normalize(input.tangent);
    
    //Gram-Schmidt orthonormalize
    input.tangent = normalize(input.tangent - input.normal * dot(input.tangent, input.normal));
    float3 bitangent = cross(input.tangent, input.normal);
    float3x3 TBNRotationMatrix = float3x3(input.tangent, bitangent, input.normal);
    
     //unpack and normalize the normal map
    float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2.0f - 1.0f;
    
    //transform the unpacked normal by the TBN matrix
    input.normal = normalize(mul(unpackedNormal, TBNRotationMatrix));
    
    //unpack the base color and adjust it by a color tint
    float3 surfaceColor = colorTint.rgb * pow(SurfaceTexture.Sample(BasicSampler, input.uv).rgb, 2.2f);
    
    //create our initial 'total light' by combining our surface color and ambient color / lighting
    float3 totalLight = surfaceColor * ambient;
    
    for (int i = 0; i < lightCount; i++)
    {
        totalLight += CreateLight(lights[i], input.normal, input.worldPosition, cameraPosition, roughness, surfaceColor);
    }
    
    return pow(float4(totalLight, 1), 1.0f / 2.2f);
    

}