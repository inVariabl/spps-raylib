#version 330

// Input vertex attributes (from vertex shader)
in vec4 fragPosition;

// Output fragment color
out vec4 finalColor;

void main()
{
    gl_FragDepth = gl_FragCoord.z;
    float depth = gl_FragCoord.z;
    finalColor = vec4(depth, depth, depth, 1.0);
}
