#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 2, binding = 0) uniform Colors{
	vec4 diffuse;
	vec4 specular;
	float opacity;
} colors;

layout(set = 2, binding = 1) uniform sampler2D diffuseTexture;
layout(set = 2, binding = 2) uniform sampler2D specularMap;
layout(set = 2, binding = 3) uniform sampler2D opacityMap;
layout(set = 2, binding = 4) uniform sampler2D normalMap;

layout(location = 0) in vec2 inUV;

// fragment shader code:
void main() 
{
	float opacity = colors.opacity * texture(opacityMap, inUV).r;
	if (opacity < 0.5f)
	{
		discard;
	}

	// gl_FragDepth = gl_FragCoord.z;
}