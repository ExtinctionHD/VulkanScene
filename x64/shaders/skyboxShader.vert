#version 450
#extension GL_ARB_separate_shader_objects : enable

// binding from application:

// uniform buffer with mvp matrices
// that positioning vertices in scene space
layout(binding = 0) uniform UBO {
	mat4 mvp;
} ubo;

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
	vec4 mvpPos = ubo.mvp * vec4(inPosition, 1.0);
    gl_Position = mvpPos.xyww;

    fragTexCoord = inPosition * vec3(1.0f, -1.0f, 1.0f);

    // fixes difference from opengl
    gl_Position.y = -gl_Position.y;
}