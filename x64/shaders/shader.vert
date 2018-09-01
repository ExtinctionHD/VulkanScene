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

// vertex input attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

// value passed to fragment shader
layout(location = 0) out vec2 fragTexCoord;

// result of vertex shader: position of each vertex
out gl_PerVertex {
    vec4 gl_Position;
};

// vertex shader code
void main() {
    gl_Position = mvp.proj * mvp.view * mvp.model * vec4(inPosition, 1.0);

    fragTexCoord = inTexCoord;
}