#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform Material{
	vec4 diffuse;
	vec4 specular;
	float opacity;
} material;

layout(set = 1, binding = 1) uniform sampler2D diffuseTexture;
layout(set = 1, binding = 2) uniform sampler2D specularMap;
layout(set = 1, binding = 3) uniform sampler2D opacityMap;
layout(set = 1, binding = 4) uniform sampler2D normalMap;

layout(location = 0) in vec2 inUV;

void main() 
{
	float opacity = material.opacity * texture(opacityMap, inUV).r;
	
	if (opacity < 0.5f)
	{
		discard;
	}

	// gl_FragDepth = gl_FragCoord.z;
}