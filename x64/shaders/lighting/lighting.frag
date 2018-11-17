#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform Lighting {
	vec3 color;
	float ambientStrength;
	vec3 direction;
	float directedStrength;
	vec3 cameraPos;
	float specularPower;
} light;

layout(binding = 1) uniform LightingSpaceMatrix {
    mat4 matrix;
} lightingSpace;

layout (binding = 2) uniform sampler2DMS posMap;
layout (binding = 3) uniform sampler2DMS normalMap;
layout (binding = 4) uniform sampler2DMS albedoMap;
layout (binding = 5) uniform sampler2DMS specularMap;
layout (binding = 6) uniform sampler2D shadowsMap;

layout (constant_id = 0) const int NUM_SAMPLES = 8;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

// Manual resolve for MSAA samples 
vec4 resolve(sampler2DMS tex, ivec2 uv)
{
	vec4 result = vec4(0.0);	   
	for (int i = 0; i < NUM_SAMPLES; i++)
	{
		vec4 val = texelFetch(tex, uv, i); 
		result += val;
	}    
	// Average resolved samples
	return result / float(NUM_SAMPLES);
}

vec3 calculateLighting(vec3 pos, vec3 normal, vec4 albedo, float specular)
{
	return vec3(0.0f);
}

void main() 
{
	ivec2 size = textureSize(posMap);
	ivec2 uv = ivec2(inUV * size);

	// // Calualte lighting for every MSAA sample
	// for (int i = 0; i < NUM_SAMPLES; i++)
	// { 
	// 	vec3 pos = texelFetch(posMap, uv, i).rgb;
	// 	vec3 normal = texelFetch(normalMap, uv, i).rgb;
	// 	vec4 albedo = texelFetch(albedoMap, uv, i);
	// 	float specular = texelFetch(specularMap, uv, i).r;

	// 	fragColor += calculateLighting(pos, normal, albedo);
	// }

	// fragColor = (alb.rgb * ambient) + fragColor / float(NUM_SAMPLES);
   
	outColor = vec4(texelFetch(posMap, uv, 0).rgb, 1.0);	
}