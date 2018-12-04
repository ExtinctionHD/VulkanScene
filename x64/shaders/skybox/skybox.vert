#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform SpaceMatrix{
    mat4 view;
    mat4 proj;
} space;

layout(set = 1, binding = 0) uniform ModelMatrix{
    mat4 matrix;
} model;

layout(location = 0) in vec3 inPos;

layout(location = 0) out vec3 outUV;

out gl_PerVertex{
    vec4 gl_Position;
};

void main() 
{	
	vec4 mvpPos = space.proj * space.view * model.matrix * vec4(inPos, 1.0);
    gl_Position = mvpPos.xyww;

    outUV = inPos;
    outUV.y = -outUV.y;
}