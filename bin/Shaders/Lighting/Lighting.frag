#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (constant_id = 0) const int SAMPLE_COUNT = 1;
layout (constant_id = 1) const int CASCADE_COUNT = 4;
layout (constant_id = 2) const float BIAS = 0.0005f;

layout (binding = 0) uniform Lighting{
	vec3 color;
	float ambientStrength;
	vec3 direction;
	float directedStrength;
	vec3 cameraPos;
	float specularPower;
} lighting;

layout (binding = 1) uniform Space{
	mat4 view;
	mat4 proj;
};

layout (binding = 2) uniform CascadeSplits{
	vec4 splits;
};

layout (binding = 3) uniform CascadeSpaces{
	mat4 viewProj[CASCADE_COUNT];
};

layout (binding = 4) uniform sampler2DMS posMap;
layout (binding = 5) uniform sampler2DMS normalMap;
layout (binding = 6) uniform sampler2DMS albedoMap;
layout (binding = 7) uniform sampler2D ssaoMap;
layout (binding = 8) uniform sampler2DArray shadowMap;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

// Manual resolve for MSAA samples 
vec4 resolve(sampler2DMS tex, ivec2 uv)
{
	vec4 result = vec4(0.0f);	   
	for (int i = 0; i < (SAMPLE_COUNT); i++)
	{
		vec4 val = texelFetch(tex, uv, i); 
		result += val;
	}    
	// Average resolved samples
	return result / float(SAMPLE_COUNT);
}

float getAmbientIntensity()
{
	return lighting.ambientStrength;
}

float getdirectIntensity(vec3 N, vec3 L)
{
	float directFactor = clamp(dot(N, L), 0.0f, 1.0f);
	return lighting.directedStrength * directFactor;
}

float getSpecularIntensity(vec3 N, vec3 L, vec3 V, float specular)
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

	return lighting.directedStrength * specular * specularFactor;
}

// 1 - fragment in the shadow, 0 - fragment in the lighting
float getShading(vec4 pos, float bias, uint cascadeIndex)
{
	// normalize proj coordiantes
    vec3 projCoords = pos.xyz / pos.w;
    projCoords = vec3(projCoords.xy * 0.5f + 0.5f, projCoords.z);

    float currentDepth = projCoords.z;
    float shadow = 0.0f;
	vec2 texelSize = 1.0f / textureSize(shadowMap, 0).xy;

	// avarage value from 9 nearest texels (PCF)
	int count = 0;
	int range = 1;
	for(int x = -range; x <= range; ++x)
	{
	    for(int y = -range; y <= range; ++y)
	    {
	        float pcfDepth = texture(shadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, cascadeIndex)).r;
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

vec3 calculateLighting(vec3 pos, vec3 N, vec3 albedo, float specular, float ssao, vec4 viewPos)
{
	vec3 L = normalize(-lighting.direction);
	vec3 V = normalize(lighting.cameraPos - pos);

	// Get cascade index for the current fragment's view position
	uint cascadeIndex = 0;
	for(uint i = 0; i < CASCADE_COUNT - 1; ++i) 
	{
		if(viewPos.z < splits[i]) 
		{
			cascadeIndex = i + 1;
		}
	}

	// Depth compare for shadowing
	vec4 shadowCoord = viewProj[cascadeIndex] * vec4(pos, 1.0f);	

	float bias = max(BIAS * (1.0f - dot(N, lighting.direction)), BIAS / 10) / (cascadeIndex + 1);
	float illumination = 1.0f - getShading(shadowCoord, bias, cascadeIndex);

	float ambientI = getAmbientIntensity() * (1.0f - ssao);
	float directI = getdirectIntensity(N, L) * illumination;
	float specularI = getSpecularIntensity(N, L, V, specular) * illumination;

	vec3 lightingComponent = lighting.color * albedo * (ambientI + directI);
	vec3 specularComponent = lighting.color * specularI;

	vec3 result = lightingComponent + specularComponent;

	// Pssm debug
	// switch(cascadeIndex) 
	// {
	// case 0: 
	// 	result *= vec3(1.0f, 0.25f, 0.25f);
	// 	break;
	// case 1: 
	// 	result *= vec3(0.25f, 1.0f, 0.25f);
	// 	break;
	// case 2: 
	// 	result *= vec3(0.25f, 0.25f, 1.0f);
	// 	break;
	// case 3: 
	// 	result *= vec3(1.0f, 1.0f, 0.25f);
	// 	break;
	// }

	return result;
}

void main() 
{
	ivec2 size = textureSize(posMap);
	ivec2 uv = ivec2(inUV * size);

	vec3 pos = resolve(posMap, uv).rgb;
	vec3 normal = resolve(normalMap, uv).rgb * 2.0f - 1.0f;
	vec4 albedoAndSpec = resolve(albedoMap, uv);
	vec3 albedo = albedoAndSpec.rgb;
	float specular = albedoAndSpec.a;
	float ssao = texture(ssaoMap, inUV).r;
	vec4 viewPos = view * vec4(pos, 1.0f);
	
	outColor = vec4(calculateLighting(pos, normal, albedo, specular, ssao, viewPos), 1.0f);
}