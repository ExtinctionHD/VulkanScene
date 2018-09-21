#version 450
#extension GL_ARB_separate_shader_objects : enable

// bindings from application:

// texture sampler with earth texture
layout(binding = 1) uniform samplerCube textureSampler;

// input and output values:

// input data obtained from vertex shader
layout(location = 0) in vec3 fragTexCoord;

// result of fragment shader: color of each fragment
layout(location = 0) out vec4 outColor;

// fragment shader code:
void main() 
{
    outColor = texture(textureSampler, fragTexCoord);
}