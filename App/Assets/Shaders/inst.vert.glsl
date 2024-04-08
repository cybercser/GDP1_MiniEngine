#version 460 core

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangents;
layout(location = 4) in vec3 a_Bitangents;
layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;
layout(location = 7) in mat4 instanceMatrix;

out VS_OUT{
    vec3 Pos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 ProjTexCoord;
} vs_out;

uniform mat4 u_View;
uniform mat4 u_Proj;
uniform mat3 u_NormalMat;
uniform mat4 u_ProjectorMat;

void main() {
    // Apply the instance-specific transformation first
    vec4 instancePos = vec4(a_Pos, 1.0);

    mat4 viewModel = u_View * instanceMatrix;
    gl_Position =  u_Proj * viewModel * instancePos;

    vs_out.TexCoords = a_TexCoords;
    vs_out.Normal = normalize(u_NormalMat * (instanceMatrix * vec4(a_Normal, 0.0)).xyz);
    vs_out.Pos = (u_View * instancePos).xyz;
    vs_out.ProjTexCoord = u_ProjectorMat * instancePos;
}
