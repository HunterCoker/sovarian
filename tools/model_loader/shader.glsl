// !vertex
#version 330 core

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;
layout(location = 5) in int a_BoneIds[4];
layout(location = 6) in float a_Weights[4];

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec2 o_TexCoord;

void main() {
    mat4 v_MVP = u_Projection * u_View * u_Model;
    gl_Position = v_MVP * vec4(a_Pos, 1.0);
    o_TexCoord = a_TexCoord;
}

// !fragment
#version 330 core

uniform sampler2D u_TextureDiffuse;
uniform sampler2D u_TextureSpecular;
uniform sampler2D u_TextureNormals;
uniform sampler2D u_TextureHeight;

in vec2 o_TexCoord;

out vec4 o_FragColor;

void main() {
    o_FragColor = texture(u_TextureDiffuse, o_TexCoord);
}
