#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (constant_id = 0) const int CASCADE_COUNT = 4;

layout(set = 0, binding = 0) uniform Space{
    mat4 viewProj[CASCADE_COUNT];
};

layout(push_constant) uniform PushConsts {
	uint cascadeIndex;
};

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 4) in mat4 transformation;

layout(location = 0) out vec2 outUV;

out gl_PerVertex{
    vec4 gl_Position;
};

void main() 
{	
	outUV = inUV;
	
    gl_Position = viewProj[cascadeIndex] * transformation * vec4(inPos, 1.0f);
}