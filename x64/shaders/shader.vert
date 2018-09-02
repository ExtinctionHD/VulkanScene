#version 450
#extension GL_ARB_separate_shader_objects : enable

// binding from application:

// uniform buffer with mvp matrices
// that positioning vertices in scene space
layout(binding = 0) uniform ModelViewProjection {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

// input and output values:

// vertex input attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

// value passed to fragment shader
layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragPos;

// result of vertex shader: position of each vertex
out gl_PerVertex {
    vec4 gl_Position;
};

// vertex shader code
void main() {
    fragTexCoord = inTexCoord;
    // vertex normal vector in world coordinates
    fragNormal = (mvp.model * vec4(inNormal, 0.0f)).xyz;	
    // position of fragment in world coordinates
    fragPos = (mvp.model * vec4(inPosition, 1.0f)).xyz;

    gl_Position = mvp.proj * mvp.view * mvp.model * vec4(inPosition, 1.0f);

    // fixes difference from opengl
    gl_Position.y = -gl_Position.y;
}