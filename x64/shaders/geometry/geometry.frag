#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 2, binding = 0) uniform Colors{
	vec4 albedo;
	vec4 specular;
	float opacity;
} colors;

layout(set = 2, binding = 1) uniform sampler2D albedoMap;
layout(set = 2, binding = 2) uniform sampler2D specularMap;
layout(set = 2, binding = 3) uniform sampler2D opacityMap;
layout(set = 2, binding = 4) uniform sampler2D normalMap;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout (location = 0) out vec4 outPos;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;

vec3 getBumpedNormal(vec3 normal, vec3 tangent, vec2 uv, sampler2D normalMap)
{
	normal = normalize(normal);

	// texture u vector in world space
	tangent = normalize(tangent);
	tangent = normalize(tangent - dot(tangent, normal) * normal);

	// texture v vector in world space
	vec3 bitangent = cross(tangent, normal);

	// normal from map
	vec3 bumMapNormal = texture(normalMap, uv).xyz;
	bumMapNormal = 2.0f * bumMapNormal - vec3(1.0f);

	// normal from map in world space
	vec3 resultNormal;
	mat3 tbn = mat3(tangent, bitangent, normal);
	resultNormal = tbn * bumMapNormal;

	return normalize(resultNormal);
}

void main() 
{
	outPos = vec4(inPos, 1.0f);

	outNormal = vec4(getBumpedNormal(inNormal, inTangent, inUV, normalMap) * 0.5f + 0.5f, 0.0f);

	vec3 albedo = colors.albedo.rgb * texture(albedoMap, inUV).rgb;
	float specular = colors.specular.r * texture(specularMap, inUV).r;
	outAlbedo = vec4(albedo, specular);
}