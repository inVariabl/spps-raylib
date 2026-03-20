#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;
in vec4 fragPosLight;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

// Shadow map uniform
uniform sampler2D shadowMap;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform float ambient;
uniform float shadowBias;

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec3 normal = normalize(fragNormal);
    vec3 lightVector = normalize(lightDir);
    float lambert = clamp(dot(normal, lightVector), 0.0, 1.0);

    // Shadow calculation
    vec3 projCoords = fragPosLight.xyz / fragPosLight.w;
    projCoords = projCoords * 0.5 + 0.5;
    projCoords.y = 1.0 - projCoords.y;

    float shadow = 0.0;
    float bias = max(shadowBias * (1.0 - lambert), 0.0005);

    bool insideShadowMap = projCoords.x >= 0.0 && projCoords.x <= 1.0 &&
                           projCoords.y >= 0.0 && projCoords.y <= 1.0 &&
                           projCoords.z >= 0.0 && projCoords.z <= 1.0;

    if (insideShadowMap)
    {
        vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
        for (int x = -1; x <= 1; ++x)
        {
            for (int y = -1; y <= 1; ++y)
            {
                vec2 sampleUv = clamp(projCoords.xy + vec2(x, y) * texelSize, 0.0, 1.0);
                float pcfDepth = texture(shadowMap, sampleUv).r;
                shadow += projCoords.z - bias > pcfDepth ? 1.0 : 0.0;
            }
        }
        shadow /= 9.0;
    }

    vec3 ambientColor = vec3(ambient);
    vec3 diffuse = lightColor * lambert;
    float visibility = 1.0 - shadow * 0.65;

    finalColor = texelColor * fragColor * colDiffuse * vec4((ambientColor + diffuse) * visibility, 1.0);
}
