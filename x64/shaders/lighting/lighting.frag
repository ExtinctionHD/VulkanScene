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
layout (binding = 5) uniform sampler2D shadowsMap;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

const int NUM_SAMPLES = 8;

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

float getAmbientIntensity()
{
	return light.ambientStrength;
}

float getDiffuseIntensity(vec3 N, vec3 L)
{
	float diffuseFactor = clamp(dot(N, L), 0.0f, 1.0f);
	return light.directedStrength * diffuseFactor;
}

float getSpecularIntensity(vec3 N, vec3 L, vec3 V, float specular)
{
	vec3 H = normalize(L + V);
	float specularFactor = dot(N, H);

	if (specularFactor > 0.0f)
	{
		specularFactor = pow(specularFactor, light.specularPower);
	}
	else
	{
		specularFactor = 0.0f;
	}

	return light.directedStrength * specular * specularFactor;
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

vec3 calculateLighting(vec3 pos, vec3 N, vec3 albedo, float specular)
{
	vec3 L = normalize(-light.direction);
	vec3 V = normalize(light.cameraPos - pos);

	vec4 fragPosLightingSpace = lightingSpace.matrix * vec4(pos, 1.0f);

	float bias = max(0.0015f * (1.0f - dot(N, light.direction)), 0.00015f);
	float illumination = 1.0f - getShading(fragPosLightingSpace, bias);

	float ambientI = getAmbientIntensity();
	float diffuseI = getDiffuseIntensity(N, L); // * illumination;
	float specularI = getSpecularIntensity(N, L, V, specular); // * illumination;

	vec3 lightingComponent = light.color * albedo * (ambientI + diffuseI);
	vec3 specularComponent = light.color * specularI;

	return lightingComponent + specularComponent;
}

void main() 
{
	vec3 fragColor = vec3(0.0f);
	ivec2 size = textureSize(posMap);
	ivec2 uv = ivec2(inUV * size);

	// Calualte lighting for every MSAA sample
	for (int i = 0; i < NUM_SAMPLES; i++)
	{ 
		vec3 pos = texelFetch(posMap, uv, i).rgb;
		vec3 normal = texelFetch(normalMap, uv, i).rgb;
		vec4 albedoAndSpec = texelFetch(albedoMap, uv, i);
		vec3 albedo = albedoAndSpec.rgb;
		float specular = albedoAndSpec.a;

		fragColor += calculateLighting(pos, normal, albedo, specular);
	}

	fragColor /= float(NUM_SAMPLES);

	outColor = vec4(fragColor, 1.0f);
}