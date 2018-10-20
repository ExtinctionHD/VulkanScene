#version 450
#extension GL_ARB_separate_shader_objects : enable

// bindings from application:

layout(set = 0, binding = 1) uniform Lighting {
	vec3 color;
	float ambientStrength;
	vec3 direction;
	float diffuseStrength;
	vec3 cameraPos;
	float specularPower;
} light;

layout(set = 2, binding = 0) uniform Colors {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float opacity;
} colors;

layout(set = 2, binding = 1) uniform sampler2D ambientTexture;
layout(set = 2, binding = 2) uniform sampler2D diffuseTexture;
layout(set = 2, binding = 3) uniform sampler2D specularMap;
layout(set = 2, binding = 4) uniform sampler2D opacityMap;
layout(set = 2, binding = 5) uniform sampler2D normalMap;

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

	vec3 ambient = light.color * light.ambientStrength * colors.ambient.rgb * texture(ambientTexture, fragTexCoord).rgb;

	vec3 diffuse = vec3(0.0f, 0.0f, 0.0f);
	vec3 specular = vec3(0.0f, 0.0f, 0.0f);

	float diffuseFactor = dot(normal, -direction);
	if (diffuseFactor > 0)
	{
		diffuse = light.color * light.diffuseStrength * diffuseFactor * colors.diffuse.rgb;

		vec3 fragToCamera = normalize(light.cameraPos - fragPos);
		vec3 lightReflect = normalize(reflect(direction, normal));
		float specularFactor = dot(fragToCamera, lightReflect);
		if (specularFactor > 0)
		{
			specular = light.color * pow(specularFactor, light.specularPower) * colors.specular.r * texture(specularMap, fragTexCoord).r;
		}
	}

	float opacity = colors.opacity * texture(opacityMap, fragTexCoord).r;
	outColor = vec4(ambient + diffuse + specular, opacity) * texture(diffuseTexture, fragTexCoord);

	// gamma correction
	//float gamma = 2.2f;	
    //outColor.rgb = pow(outColor.rgb, vec3(1.0f / gamma));
}