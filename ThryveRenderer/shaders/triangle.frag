#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexColor;
// Interpolated color from vertex shader
layout(location = 0) out vec4 outColor; // Final color output

void main() {
    outColor = vec4((fragTexColor),0.0,1.0); // Use the interpolated color
}