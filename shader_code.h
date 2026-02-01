#pragma once
const char* phongVertex = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 mvp;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec4 FragPosLightSpace;

void main()
{
    gl_Position = mvp * vec4(aPos, 1.0);
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoord;
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
}
)";

const char* phongFragment = R"(

#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

uniform sampler2D tex;
uniform sampler2D shadowMap;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    // Perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // Outside depth range of light
    if (projCoords.z > 1.0)
        return 0.0;

    // ===== THIS IS THE CHECK YOU ASKED FOR =====
    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0)
    {
        return 0.0;
    }
    // ==========================================

    float currentDepth = projCoords.z;

    // Bias to reduce shadow acne
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    // Percentage-closer filtering (3x3)
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(
                shadowMap,
                projCoords.xy + vec2(x, y) * texelSize
            ).r;

            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir  = normalize(viewPos - FragPos);

    // Ambient
    vec3 ambient = 0.1 * lightColor;

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Blinn-Phong specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 64.0);
    vec3 specular = 0.5 * spec * lightColor;

    float shadow = ShadowCalculation(FragPosLightSpace, norm, lightDir);

    vec3 objectColor = texture(tex, TexCoords).rgb;

    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);
    vec3 result = lighting * objectColor;

    FragColor = vec4(result, 1.0);
}

)";