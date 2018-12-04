#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 2) uniform Lighting{
	vec3 color;
	float ambientStrength;
	vec3 direction;
	float directedStrength;
	vec3 cameraPos;
	float specularPower;
} lighting;

layout(set = 0, binding = 3) uniform sampler2D shadowsMap;

layout(set = 2, binding = 0) uniform Colors{
	vec4 diffuse;
	vec4 specular;
	float opacity;
} colors;

layout(set = 2, binding = 1) uniform sampler2D diffuseTexture;
layout(set = 2, binding = 2) uniform sampler2D specularMap;
layout(set = 2, binding = 3) uniform sampler2D opacityMap;
layout(set = 2, binding = 4) uniform sampler2D normalMap;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec4 inPosInLightSpace;

layout(location = 0) out vec4 outColor;

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
	bumMapNormal = 2.0f * bumMapNormal - vec3(1.0f, 1.0f, 1.0f);

	// normal from map in world space
	vec3 resultNormal;
	mat3 tbn = mat3(tangent, bitangent, normal);
	resultNormal = tbn * bumMapNormal;
	resultNormal = normalize(resultNormal);

	return resultNormal;
}

float getAmbientIntensity()
{
	return lighting.ambientStrength;
}

float getDiffuseIntensity(vec3 N, vec3 L)
{
	float diffuseFactor = clamp(dot(N, L), 0.0f, 1.0f);
	return lighting.directedStrength * diffuseFactor;
}

float getSpecularIntensity(vec3 N, vec3 L, vec3 V)
{
	vec3 H = normalize(L + V);
	float specularFactor = dot(N, H);

	if (specularFactor > 0.0f)
	{
		specularFactor = pow(specularFactor, lighting.specularPower);
	}
	else
	{
		specularFactor = 0.0f;
	}

	return lighting.directedStrength * colors.specular.r * texture(specularMap, inUV).r * specularFactor;
}

// 1 - fragment in the shadow, 0 - fragment in the lighting
float getShading(vec4 inPosLightSpace, float bias)
{
	// normalize proj coordiantes
    vec3 projCoords = inPosLightSpace.xyz / inPosLightSpace.w;
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

const float BIAS_FACTOR = 0.001f;
const float MIN_BIAS = 0.0001f;

// fragment shader code:
void main() 
{
	vec3 L = normalize(-lighting.direction);
	vec3 V = normalize(lighting.cameraPos - inPos);
	vec3 N = getBumpedNormal(inNormal, inTangent, inUV, normalMap);

	float bias = max(BIAS_FACTOR * (1.0f - dot(N, lighting.direction)), MIN_BIAS);
	float illumination = 1.0f - getShading(inPosInLightSpace, bias);

	float ambientI = getAmbientIntensity();
	float diffuseI = getDiffuseIntensity(N, L) * illumination;
	float specularI = getSpecularIntensity(N, L, V) * illumination;

	vec3 diffuseColor = colors.diffuse.rgb * texture(diffuseTexture, inUV).rgb;

	vec3 lightingComponent = lighting.color * diffuseColor * (ambientI + diffuseI);
	vec3 specularComponent = lighting.color * specularI;

	float opacity = colors.opacity * texture(opacityMap, inUV).r;

	outColor = vec4(lightingComponent + specularComponent, opacity);

	// gamma correction
	// float gamma = 2.2f;	
	// outColor.rgb = pow(outColor.rgb, vec3(1.0f / gamma));
}