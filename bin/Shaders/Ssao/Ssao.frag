#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (constant_id = 0) const uint SSAO_KERNEL_SIZE = 32;
layout (constant_id = 1) const float SSAO_RADIUS = 0.4f;
layout (constant_id = 2) const float SSAO_POWER = 1.0f;

layout (binding = 0) uniform SsaoKernel{
	vec4 kernel[SSAO_KERNEL_SIZE];
};

layout (binding = 1) uniform Space{
    mat4 view;
    mat4 proj;
};

layout (binding = 2) uniform sampler2DMS posMap;
layout (binding = 3) uniform sampler2DMS normalMap;
layout (binding = 4) uniform sampler2D noiseTexture;

layout (location = 0) in vec2 inUV;

layout (location = 0) out float outColor;

const float BIAS = 0.0001f;

void main() 
{
	ivec2 dim = textureSize(posMap);
	ivec2 uv = ivec2(inUV * dim);

	vec3 pos = vec3(view * texelFetch(posMap, uv, 0));
	vec3 normal = vec3(view * vec4(texelFetch(normalMap, uv, 0).rgb * 2.0f - 1.0f, 0.0f));

	ivec2 noiseDim = textureSize(noiseTexture, 0);
	vec2 noiseUV = vec2(float(dim.x) / float(noiseDim.x), float(dim.y) / (noiseDim.y)) * inUV;  

	vec3 randomVec = texture(noiseTexture, noiseUV).xyz;

	// Create TBN matrix
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(tangent, normal);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0f;
	for(int i = 0; i < SSAO_KERNEL_SIZE; ++i)
	{
		vec3 samplePos = TBN * kernel[i].xyz; 
	    samplePos = pos + samplePos * SSAO_RADIUS;

	    vec4 offset = vec4(samplePos, 1.0f);
		offset = proj * offset;
		offset.xyz /= offset.w;
		offset.xy = offset.xy * 0.5f + 0.5f;

		float sampleDepth = vec4(view * texelFetch(posMap, ivec2(offset.xy * dim), 0)).z;

		float rangeCheck = smoothstep(0.0f, 1.0f, SSAO_RADIUS / abs(pos.z - sampleDepth));
		occlusion += (sampleDepth >= samplePos.z + BIAS ? 1.0 : 0.0) * rangeCheck;
	}
	occlusion = occlusion / float(SSAO_KERNEL_SIZE);
	occlusion = pow(occlusion, SSAO_POWER);

	outColor = occlusion;
}