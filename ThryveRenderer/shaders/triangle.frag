#version 450
layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
// Interpolated color from vertex shader
layout(location = 0) out vec4 outColor; // Final color output

void main() {
    outColor = texture(texSampler,fragTexCoord); // Use the interpolated color
}