#version 460 core

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;

out VS_OUT{
	vec3 Pos;
	vec3 Normal;
} vs_out;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;
uniform mat3 u_NormalMat;

void main() {
	vs_out.Normal = normalize(u_NormalMat * a_Normal);
	vs_out.Pos = (u_View * u_Model * vec4(a_Pos, 1.0)).xyz;

	gl_Position = u_Proj * u_View * u_Model * vec4(a_Pos, 1.0);
}
