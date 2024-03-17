#version 450

layout(binding = 0) uniform UniformBufferObject{
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

layout(location = 0) in vec2 inPosition; //Vertex Color

layout(location = 1) in vec3 inColor; //Vertex Color

layout(location = 0) out vec3 fragColor; // Pass the Color to fragment Shader

void main() {
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0f);
    fragColor = inColor; // Pass the vertex Color to the fragment shader
}
