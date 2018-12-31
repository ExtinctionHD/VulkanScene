#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (constant_id = 0) const int BLUR_RANGE = 1;

layout (binding = 0) uniform sampler2D ssaoMap;

layout (location = 0) in vec2 inUV;

layout (location = 0) out float outColor;

void main() 
{
	int n = 0;
	vec2 texelSize = 1.0 / vec2(textureSize(ssaoMap, 0));
	float result = 0.0;
	for (int x = -BLUR_RANGE; x < BLUR_RANGE; x++) 
	{
		for (int y = -BLUR_RANGE; y < BLUR_RANGE; y++) 
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(ssaoMap, inUV + offset).r;
			n++;
		}
	}
	outColor = result / (float(n));
	
	// no blur
	// outColor = texture(ssaoMap, inUV).r;
}