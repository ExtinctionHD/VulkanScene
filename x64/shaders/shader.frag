#version 450
#extension GL_ARB_separate_shader_objects : enable

// bindings from application:

// uniform buffer with lighting attributes
// (color and strength of ambient light)
layout(binding = 1) uniform Lighting {
	vec3 color;
	float ambientStrength;
	vec3 direction;
} light;

// texture sampler with earth texture
layout(binding = 2) uniform sampler2D textureSampler;

// normal map
layout(binding = 3) uniform sampler2D normalMap;

// input and output values:

// input data obtained from vertex shader
layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;

// result of fragment shader: color of each fragment
layout(location = 0) out vec4 outColor;

// fragment shader code:
void main() 
{
	vec3 direction = normalize(light.direction);

	// diffuse lighting
	float diffuseFactor = max(dot(normalize(fragNormal), -direction), 0.0);
	vec3 diffuse = light.color * diffuseFactor;;

	// ambient lighting
	vec3 ambient = light.color * light.ambientStrength;

	// result lighting
	vec3 result = (ambient + diffuse);

	// fragment color without lighting
	outColor = texture(textureSampler, fragTexCoord);

    outColor = vec4(result * outColor.rgb, 1.0f);
}