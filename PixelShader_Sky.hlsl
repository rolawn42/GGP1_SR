#include "ShaderIncludes.hlsli"

TextureCube SkyBox : register(t0);
SamplerState BasicSampler : register(s0);

float4 main(VertexToPixel_Sky input) : SV_TARGET
{
    return SkyBox.Sample(BasicSampler, input.sampleDirection);
}