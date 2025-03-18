cbuffer psConstantBuffer : register(b0)
{
    float4 colorTint;
}

// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 screenPosition	: SV_POSITION;
    float2 uv				: TEXCOORD; // UV texture coordinates
    float3 normal			: NORMAL; // RGB normal map
};

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
	
	//I'm trying to create a simple black and red checkered pattern.
    
	//forces sine output to a 0 - 1 mask on xy by scaling from 1 - -1 to .5 - -.5 the results of the sine on the uv
	float2 mask = ceil(sin(input.uv * 10.0f) / 2.0f);
	//for the XY values lerps a color with the previously defined mask value
	
	//CHANGED: I wasn't quite doing the math right on this before, I used the lerped mask values 
	//incorrectly by not combining them to make the 'one true mask' as it were
    return float4((lerp(1.0f, 0.0f, mask.x) + lerp(1.0f, 0.0f, mask.y)) % 2.0f, 0.0f, 0.0f, 0.0f);
}