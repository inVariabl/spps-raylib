#version 330

// Input vertex attributes
in vec3 vertexPosition;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;

// Output vertex attributes (to fragment shader)
out vec4 fragPosition;

void main()
{
    // Calculate final vertex position
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
