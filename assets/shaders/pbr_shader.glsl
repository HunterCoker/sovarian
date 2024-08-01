// !vertex
#version 330 core

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform vec3 u_LightPosition;

out vec3 o_Normal;
out vec2 o_TexCoord;
out vec3 o_LightPosition;

void main() {
    // gl_Position = u_Projection * u_View * u_Model * vec4(a_Pos, 1.0);
    gl_Position = vec4(a_Pos, 1.0);
    o_Normal = a_Normal;
    o_TexCoord = a_TexCoord;
    o_LightPosition = u_LightPosition;
}

// !fragment
#version 330 core

uniform sampler2D u_Texture;

in vec3 o_Normal;
in vec2 o_TexCoord;

in vec3 o_LightPosition;

out vec4 FragColor;

void main() {
    // FragColor = texture(u_Texture, o_TexCoord);
    FragColor = vec4(1.0, 0.0, 1.0, 1.0);
}
