#include "ShaderIncludes.hlsli"

cbuffer vsConstantBuffer : register(b0)
{
    matrix view;
    matrix proj;
}

VertexToPixel_Sky main( VertexShaderInput input )
{
    VertexToPixel_Sky output;
    
    //set translation to 0
    matrix newView = view;
    
    newView._14 = 0;
    newView._24 = 0;
    newView._34 = 0;
    
    output.position = mul(mul(proj, newView), float4(input.localPosition, 1.0f));
    output.position.z = output.position.w;
    
    output.sampleDirection = input.localPosition;
    
    return output;
}