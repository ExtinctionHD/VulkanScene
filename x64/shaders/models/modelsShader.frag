#version 450
#extension GL_ARB_separate_shader_objects : enable

// bindings from application:

layout(set = 0, binding = 2) uniform Lighting {
	vec3 color;
	float ambientStrength;
	vec3 direction;
	float diffuseStrength;
	vec3 cameraPos;
	float specularPower;
} light;

layout(set = 0, binding = 3) uniform sampler2D shadowsMap;

layout(set = 2, binding = 0) uniform Colors {
	vec4 diffuse;
	vec4 specular;
	float opacity;
} colors;

layout(set = 2, binding = 1) uniform sampler2D diffuseTexture;
layout(set = 2, binding = 2) uniform sampler2D specularMap;
layout(set = 2, binding = 3) uniform sampler2D opacityMap;
layout(set = 2, binding = 4) uniform sampler2D normalMap;

// input and output values:

// input data obtained from vertex shader
layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragTangent;
layout(location = 4) in vec4 fragPosLightingSpace;

// result of fragment shader: color of each fragment
layout(location = 0) out vec4 outColor;

vec3 getBumpedNormal()
{
	vec3 normal = normalize(fragNormal);

	// texture u vector in world space
	vec3 tangent = normalize(fragTangent);
	tangent = normalize(tangent - dot(tangent, normal) * normal);

	// texture v vector in world space
	vec3 bitangent = cross(tangent, normal);

	// normal from map
	vec3 bumMapNormal = texture(normalMap, fragTexCoord).xyz;
	bumMapNormal = 2.0f * bumMapNormal - vec3(1.0f, 1.0f, 1.0f);

	// normal from map in world space
	vec3 resultNormal;
	mat3 tbn = mat3(tangent, bitangent, normal);
	resultNormal = tbn * bumMapNormal;
	resultNormal = normalize(resultNormal);

	return resultNormal;
}

vec3 getAmbientIntensity()
{
	return light.color * light.ambientStrength;
}

vec3 getDiffuseIntensity(vec3 N, vec3 L)
{
	float diffuseFactor = clamp(dot(N, L), 0.0f, 1.0f);
	return light.color * light.diffuseStrength * diffuseFactor;
}

vec3 getSpecularIntensity(vec3 N, vec3 L, vec3 V)
{
	float specularFactor = 0.0f;

	if (dot(N, L) > 0.0f)
	{
		vec3 H = normalize(L + V);
		specularFactor = pow(dot(N, H), light.specularPower);
	}

	return light.color * colors.specular.r * texture(specularMap, fragTexCoord).r * specularFactor;
}

// 1 - fragment in the shadow, 0 - fragment in the light
float getShading(vec4 fragPosLightSpace, float bias)
{
	// normalize proj coordiantes
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = vec3(projCoords.xy * 0.5f + 0.5f, projCoords.z);

    float currentDepth = projCoords.z;
    float shadow = 0.0f;
	vec2 texelSize = 1.0f / textureSize(shadowsMap, 0);

	// avarage value from 9 nearest texels (PCF)
	int count = 0;
	int range = 1;
	for(int x = -range; x <= range; ++x)
	{
	    for(int y = -range; y <= range; ++y)
	    {
	        float pcfDepth = texture(shadowsMap, projCoords.xy + vec2(x, y) * texelSize).r;
	        shadow += currentDepth - bias > pcfDepth ? 1.0f : 0.0f;
	        count++;
	    }
	}
	shadow /= count;

	if(projCoords.z > 1.0)
	{
        shadow = 0.0;
	}

    return shadow;
}

// fragment shader code:
void main() 
{
	vec3 L = normalize(-light.direction);
	vec3 V = normalize(light.cameraPos - fragPos);
	vec3 N = getBumpedNormal();

	float bias = max(0.002f * (1.0f - dot(N, light.direction)), 0.0002f);
	float illumination = 1.0f - getShading(fragPosLightingSpace, bias);

	vec3 ambientI = getAmbientIntensity();
	vec3 diffuseI = getDiffuseIntensity(N, L) * illumination;
	vec3 specularI = getSpecularIntensity(N, L, V) * illumination;

	vec3 diffuseColor = colors.diffuse.rgb * texture(diffuseTexture, fragTexCoord).rgb;
	vec3 resultI = ambientI + diffuseI + specularI;

	float opacity = colors.opacity * texture(opacityMap, fragTexCoord).r;
	outColor = vec4(resultI * diffuseColor, opacity);

	// gamma correction
	// float gamma = 2.2f;	
	// outColor.rgb = pow(outColor.rgb, vec3(1.0f / gamma));
}