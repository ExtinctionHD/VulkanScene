#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (constant_id = 0) const int SAMPLE_COUNT = 1;
layout (constant_id = 1) const int CASCADE_COUNT = 4;

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
	float splits[CASCADE_COUNT];
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
float getShading(vec4 posInLightSpace, float bias)
{
	// normalize proj coordiantes
    vec3 projCoords = posInLightSpace.xyz / posInLightSpace.w;
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
	        float pcfDepth = texture(shadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, 0)).r;
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

float textureProj(vec4 P, vec2 offset, float bias, uint cascadeIndex)
{
	float shadow = 0.0f;
	vec4 shadowCoord = P / P.w;

	if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) 
	{
		float dist = texture(shadowMap, vec3(shadowCoord.st + offset, cascadeIndex)).r;
		if (shadowCoord.w > 0 && dist < shadowCoord.z - bias) 
		{
			shadow = 1.0f;
		}
	}

	return shadow;
}

float filterPCF(vec4 sc, float bias, uint cascadeIndex)
{
	ivec2 texDim = textureSize(shadowMap, 0).xy;
	float scale = 0.75f;
	float dx = scale * 1.0f / float(texDim.x);
	float dy = scale * 1.0f / float(texDim.y);

	float shadowFactor = 0.0f;
	int count = 0;
	int range = 1;
	
	for (int x = -range; x <= range; x++) 
	{
		for (int y = -range; y <= range; y++) 
		{
			shadowFactor += textureProj(sc, vec2(dx*x, dy*y), bias, cascadeIndex);
			count++;
		}
	}

	return shadowFactor / count;
}

const float BIAS_FACTOR = 0.001f;
const float MIN_BIAS = 0.0001f;

const mat4 BIAS_MAT = mat4( 
	0.5f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.5f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.5f, 0.5f, 0.0f, 1.0f 
);

vec3 calculateLighting(vec3 pos, vec3 N, vec3 albedo, float specular, float ssao, vec4 viewPos)
{
	vec3 L = normalize(-lighting.direction);
	vec3 V = normalize(lighting.cameraPos - pos);

	// Get cascade index for the current fragment's view position
	uint cascadeIndex = 0;
	for(uint i = 0; i < CASCADE_COUNT - 1; ++i) 
	{
		if(viewPos.z < -splits[i]) 
		{
			cascadeIndex = i + 1;
		}
	}

	// Depth compare for shadowing
	vec4 shadowCoord = (BIAS_MAT * viewProj[cascadeIndex]) * vec4(pos, 1.0f);	

	float bias = max(BIAS_FACTOR * (1.0f - dot(N, lighting.direction)), MIN_BIAS);
	float illumination = 1.0f - filterPCF(shadowCoord / shadowCoord.w, bias, cascadeIndex);

	float ambientI = getAmbientIntensity() * (1.0f - ssao);
	float directI = getdirectIntensity(N, L) * illumination;
	float specularI = getSpecularIntensity(N, L, V, specular) * illumination;

	vec3 lightingComponent = lighting.color * albedo * (ambientI + directI);
	vec3 specularComponent = lighting.color * specularI;

	return lightingComponent + specularComponent;
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