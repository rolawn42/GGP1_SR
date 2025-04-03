#ifndef __GGP_SHADER_INCLUDES__ 
#define __GGP_SHADER_INCLUDES__

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

#define MAX_SPECULAR_EXPONENT 256.0f

#define MAX_LIGHTS 10

// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float3 localPosition    : POSITION; // XYZ position
    float2 uv               : TEXCOORD; // UV texture coordinates
    float3 normal           : NORMAL; // RGB normal map
    float3 tangent          : TANGENT; //UV tangents
};

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
    float3 worldPosition	: POSITION;
    float3 tangent          : TANGENT;
};

struct VertexToPixel_Sky
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float4 position         : SV_POSITION;
    float3 sampleDirection  : DIRECTION;
};

//------------------------------------------------------ LIGHTING --------------------------------------------------

struct Light
{
    int type; // Which kind of light? 0, 1 or 2 (see above)
    float3 direction; // Directional and Spot lights need a direction
    float range; // Point and Spot lights have a max range for attenuation
    float3 position; // Point and Spot lights have a position in space
    float intensity; // All lights need an intensity
    float3 color; // All lights need a color
    float spotInnerAngle; // Inner cone angle (in radians) – Inside this, full light!
    float spotOuterAngle; // Outer cone angle (radians) – Outside this, no light!
    float2 padding; // Purposefully padding to hit the 16-byte boundary
};

float Attenuation(Light light, float3 worldPosition)
{
	// Calculate the distance between the current pixel and the light
    float dist = distance(light.position, worldPosition);

	// Calculates the attenution via with non-linear falloff by squaring dist and range, and then clamping it to 1
    float att = saturate(1.0f - (dist * dist / (light.range * light.range)));

    return att * att; //multipling 0-1 values creates the expoential falloff
}

float SpotTerm(Light light, float3 toLightDirection)
{
	//angle of the pixel's normal to the center of the light's direction vector
    float pixelAngle = saturate(dot(-toLightDirection, light.direction));
	
	//use cosine to calculate the inner and out circles of the fallout range
    float cosOuter = cos(light.spotOuterAngle);
    float cosInner = cos(light.spotInnerAngle);
    float falloffRange = cosOuter - cosInner;
	
	//determine the amount of adjustment based on the pixels angle in the range
    float spotTerm = saturate((cosOuter - pixelAngle) / falloffRange);

	// return the spot term
    return spotTerm;
}

float3 CreateLight(Light light, float3 normal, float3 worldPosition, float3 cameraPosition, float roughness, float3 surfaceColor)
{
    //directional: get the direction to the light and the camera from the current pixel / 
    //point-spot: get the difference from this pixels position to the lights positions
    float3 toLightDirection = normalize(lerp(-light.direction, light.position - worldPosition, saturate((float) light.type)));
    //float3 toLightDirection = light.type == LIGHT_TYPE_DIRECTIONAL ? normalize(-light.direction) : normalize(light.position - worldPosition);
    float3 toCameraDirection = normalize(cameraPosition - worldPosition);
    
    //calculate the difuse by taking the dot of the normal and the to light direction (returns their similarity, clamped to 0-1 by saturate)
    float diffuse = saturate(dot(normal, toLightDirection));
    
    //calculate the specular (first, if roughness is 1.0 the first is 0 (or no reflection)
    float specular = roughness >= 0.975f ? 0.0f :
            pow(max(dot(toCameraDirection, reflect(-toLightDirection, normal)), 0), (1 - roughness) * MAX_SPECULAR_EXPONENT);
            //otherwise, the dot of the refl vector and cam direction determines where the light reflects, max avoids negatives, 
            //and the power creates the sharpness of the reflection (size and brightness))
    
    specular *= any(diffuse);
    
    //the fundemental equation for all lights
    #define BASIC_LIGHT light.intensity * light.color * (diffuse * surfaceColor + specular)
    
    switch (light.type)
    {
        case LIGHT_TYPE_POINT:
            return Attenuation(light, worldPosition) * BASIC_LIGHT;
        case LIGHT_TYPE_SPOT:
            return Attenuation(light, worldPosition) * SpotTerm(light, toLightDirection) * BASIC_LIGHT;
        case LIGHT_TYPE_DIRECTIONAL:
        default:
            return BASIC_LIGHT;
    }
}
#endif