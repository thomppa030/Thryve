#version 450

layout(location = 0) in vec3 aPos;         // Vertex position
layout(location = 1) in vec3 aNormal;      // Vertex normal
layout(location = 2) in vec2 aTexCoord;    // Vertex texture coordinate

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

layout(location = 0) out vec2 TexCoords;
layout(location = 1) out vec3 FragPos;
layout(location = 2) out mat3 TBN;

void main()
{
    // Transform vertex position to world space
    FragPos = vec3(ubo.model * vec4(aPos, 1.0));

    // Pass texture coordinates to fragment shader
    TexCoords = aTexCoord;

    // Transform normal, tangent, and bitangent to world space
    vec3 T = normalize(mat3(ubo.model) * aTangent);
    vec3 B = normalize(mat3(ubo.model) * aBitangent);
    vec3 N = normalize(mat3(ubo.model) * aNormal);
    TBN = mat3(T, B, N);

    // Output the transformed position
    gl_Position = ubo.projection * ubo.view * vec4(FragPos, 1.0);
}



