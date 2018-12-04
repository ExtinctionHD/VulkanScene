#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform SpaceMatrix{
    mat4 view;
    mat4 proj;
} space;

layout(set = 0, binding = 1) uniform LightSpaceMatrix{
    mat4 view;
    mat4 proj;
} lightSpace;

layout(set = 1, binding = 0) uniform ModelMatrix{
	mat4 matrix;
} model;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec3 outPos;
layout(location = 1) out vec2 outUV;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec3 outTangent;
layout(location = 4) out vec4 outPosInLightSpace;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() 
{	
    // position of outment in world coordinates
    outPos = vec3(model.matrix * vec4(inPos, 1.0f));
    // texture coordinates without changes
    outUV = inUV;
    // vertex normal vector in world coordinates
    outNormal = vec3(model.matrix * vec4(inNormal, 0.0f));
    // tangent vector of vertex in world coordinates
    outTangent = vec3(model.matrix * vec4(inTangent, 0.0f));
    // position of outment in lighting space
    outPosInLightSpace = lightSpace.proj * lightSpace.view * model.matrix * vec4(inPos, 1.0f);
    
    gl_Position = space.proj * space.view * model.matrix * vec4(inPos, 1.0f);

    // fixes difference from opengl
    // gl_Position.y = -gl_Position.y;
}