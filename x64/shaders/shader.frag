#version 450
#extension GL_ARB_separate_shader_objects : enable

// binding from application:
// uniform buffer with lighting attributes
// (color and strength of ambient light)
layout(binding = 1) uniform Lighting {
	vec3 color;
	float ambientStrength;
} light;

// binding from application:
// texture sampler with earth texture
layout(binding = 2) uniform sampler2D textureSampler;

// input data obtained from vertex shader
layout(location = 0) in vec2 fragTexCoord;

// result of fragment shader: color of each fragment
layout(location = 0) out vec4 outColor;

// fragment shader code:
void main() 
{
	vec3 ambient = light.color * light.ambientStrength;	// ambient light color
	vec3 texture = texture(textureSampler, fragTexCoord).xyz * ambient;	// color of texture with ambient light

	// result color
    outColor = vec4(texture, 1.0f);
}