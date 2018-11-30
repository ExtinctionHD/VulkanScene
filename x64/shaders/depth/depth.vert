#version 450
#extension GL_ARB_separate_shader_objects : enable

// binding from application:

layout(set = 0, binding = 0) uniform SpaceMatrix{
    mat4 view;
    mat4 proj;
} space;

layout(set = 1, binding = 0) uniform ModelMatrix {
	mat4 matrix;
} model;

// vertex input attributes
layout(location = 0) in vec3 inPosition;

out gl_PerVertex {
    vec4 gl_Position;
};

// vertex shader code
void main() 
{	
    gl_Position = space.proj * space.view * model.matrix * vec4(inPosition, 1.0f);

    // fix difference from opengl
    // gl_Position.y = -gl_Position.y;
}