#version 450
#extension GL_ARB_separate_shader_objects : enable

// bindings from application:

// uniform buffer with lighting attributes
// (color and strength of ambient light)
layout(binding = 1) uniform Lighting {
	vec3 color;
	float ambientStrength;
	vec3 direction;
	float diffuseStrength;
	vec3 cameraPos;
	float specularPower;
} light;

// texture sampler with earth texture
layout(binding = 2) uniform sampler2D textureSampler;

// normal map
layout(binding = 3) uniform sampler2D normalMap;

// specular map
layout(binding = 4) uniform sampler2D specularMap;

// input and output values:

// input data obtained from vertex shader
layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragTangent;

// result of fragment shader: color of each fragment
layout(location = 0) out vec4 outColor;

vec3 calculateBumpedNormal()
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

// fragment shader code:
void main() 
{
	vec3 direction = normalize(light.direction);
	vec3 normal = calculateBumpedNormal();

	// ambient lighting
	vec3 ambient = light.color * light.ambientStrength;

	// diffuse and specular lighting
	vec3 diffuse = vec3(0.0f, 0.0f, 0.0f);
	vec3 specular = vec3(0.0f, 0.0f, 0.0f);
	float diffuseFactor = dot(normal, -direction);
	if (diffuseFactor > 0)
	{
		diffuse = light.color * diffuseFactor;

		vec3 fragToCamera = normalize(light.cameraPos - fragPos);
		vec3 lightReflect = normalize(reflect(direction, normal));
		float specularFactor = dot(fragToCamera, lightReflect);
		
		if (specularFactor > 0)
		{
			specularFactor = pow(specularFactor, light.specularPower);
			specular = light.color * texture(specularMap, fragTexCoord).rgb * specularFactor;
		}
	}

	// result lighting
	vec3 result = (ambient + diffuse + specular);

	// fragment color without lighting
	outColor = texture(textureSampler, fragTexCoord);

    outColor = vec4(result * outColor.rgb, 1.0f);
}