#version 450
#extension GL_ARB_separate_shader_objects : enable

// binding from application:

// uniform buffer with mvp matrices
// that positioning vertices in scene space
layout(set = 1, binding = 0) uniform ModelViewProjection {
	mat4 model;
	mat4 view;
	mat4 proj;
} mvp;

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
	vec4 mvpPos = mvp.proj * mvp.view * mvp.model * vec4(inPosition, 1.0);
    gl_Position = mvpPos.xyww;

    fragTexCoord = inPosition;
    fragTexCoord.y = -fragTexCoord.y;

    // fixes difference from opengl
    gl_Position.y = -gl_Position.y;
}