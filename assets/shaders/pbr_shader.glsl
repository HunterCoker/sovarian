!vertex
#version 330 core

layout (location = 0) in vec3 a_Pos;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

uniform vec3 u_LightPosition;
out vec3 o_LightPosition;

void main()
{
    gl_Position = vec4(a_Pos, 1.0);
    o_LightPosition = u_LightPosition;
}

!fragment
#version 330 core

in vec3 o_LightPosition;

out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 0.0, 1.0, 1.0);
}
