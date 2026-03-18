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
    vec3 lightDot = vec3(clamp(dot(fragNormal, lightDir), 0.0, 1.0));
    vec3 normal = normalize(fragNormal);

    // Shadow calculation
    vec3 projCoords = fragPosLight.xyz / fragPosLight.w;
    projCoords = projCoords * 0.5 + 0.5;

    float shadow = 0.0;
    float bias = max(shadowBias * (1.0 - dot(normal, lightDir)), 0.0005);

    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += projCoords.z - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;

    // Keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0) shadow = 0.0;

    vec3 ambientColor = vec3(ambient);
    vec3 diffuse = lightColor * lightDot * (1.0 - shadow);

    finalColor = texelColor * fragColor * colDiffuse * vec4(ambientColor + diffuse, 1.0);
}
