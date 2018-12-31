#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform Colors{
	vec4 diffuse;
	vec4 specular;
	float opacity;
} colors;

layout(set = 1, binding = 1) uniform samplerCube diffuseTexture;

layout(location = 0) in vec3 inUV;

layout(location = 0) out vec4 outColor;

void main() 
{
    outColor = texture(diffuseTexture, inUV);
}