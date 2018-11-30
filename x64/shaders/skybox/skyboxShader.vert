#version 450
#extension GL_ARB_separate_shader_objects : enable

// binding from application:

// uniform buffer with mvp matrices
// that positioning vertices in scene space
layout(set = 0, binding = 0) uniform SpaceMatrix {
    mat4 view;
    mat4 proj;
} space;

layout(set = 1, binding = 0) uniform ModelMatrix {
    mat4 matrix;
} model;

// input and output values:

// vertex input attributes
layout(location = 0) in vec3 inPosition;

// value passed to fragment shader
layout(location = 0) out vec3 fragTexCoord;

// result of vertex shader: position of each vertex
out gl_PerVertex {
    vec4 gl_Position;
};

// vertex shader code
void main() 
{	
	vec4 mvpPos = space.proj * space.view * model.matrix * vec4(inPosition, 1.0);
    gl_Position = mvpPos.xyww;

    fragTexCoord = inPosition;
    fragTexCoord.y = -fragTexCoord.y;

    // fixes difference from opengl
    // gl_Position.y = -gl_Position.y;
}