#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    float posX;
    float posY;
    float size;
} ubo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() 
{
    gl_Position = vec4(ubo.size*inPosition.x+ubo.posX, ubo.size*inPosition.y+ubo.posY, 0.0, 1.0);
    fragColor = inColor;
}