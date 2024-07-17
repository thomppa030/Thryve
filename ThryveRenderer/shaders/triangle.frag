#version 450

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec2 TexCoords;
layout(location = 1) in vec3 FragPos;
layout(location = 2) in mat3 TBN;

layout(binding = 1) uniform sampler2D albedoMap;
layout(binding = 2) uniform sampler2D normalMap;
layout(binding = 3) uniform sampler2D metallicMap;
layout(binding = 4) uniform sampler2D emissionMap;

// Define the light and view positions
const vec3 lightPos = vec3(10.0, 10.0, 10.0);
const vec3 viewPos = vec3(0.0, 0.0, 10.0);

void main()
{
    // Obtain normal from normal map in tangent space
    vec3 normal = texture(normalMap, TexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0); // Transform from [0,1] to [-1,1]

    // Transform normal to world space
    normal = normalize(TBN * normal);

    // Calculate lighting
    vec3 lightColor = vec3(1.0);
    vec3 ambient = 0.1 * lightColor;

    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = spec * lightColor;

    // Obtain albedo color
    vec3 albedo = texture(albedoMap, TexCoords).rgb;

    // Obtain metallic factor
    float metallic = texture(metallicMap, TexCoords).r;

    // Combine diffuse and specular based on metallic factor
    vec3 color = (ambient + diffuse * (1.0 - metallic) + specular * metallic) * albedo;

    // Obtain emission color
    vec3 emission = texture(emissionMap, TexCoords).rgb;

    // Add emission to final color
    color += emission;

    // Output final color
    FragColor = vec4(color, 1.0);
}
