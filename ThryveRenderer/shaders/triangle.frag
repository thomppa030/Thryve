#version 450

layout(location = 0) in vec3 fragColor; // Interpolated color from vertex shader
layout(location = 0) out vec4 outColor; // Final color output

void main() {
    outColor = vec4((fragColor),1.0); // Use the interpolated color
}