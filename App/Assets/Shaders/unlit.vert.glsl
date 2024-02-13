#version 460 core

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;

out VS_OUT{
	vec2 TexCoords;
} vs_out;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;

void main() {
	vs_out.TexCoords = a_TexCoords;
	gl_Position = u_Proj * u_View * u_Model * vec4(a_Pos, 1.0);
}
