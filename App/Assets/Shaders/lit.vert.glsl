#version 460 core

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 5) in ivec4 a_BoneIds;     // INT pointer
layout(location = 6) in vec4 a_Weights;

out VS_OUT{
	vec3 Pos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 ProjTexCoord;
} vs_out;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;
uniform mat3 u_NormalMat;

uniform mat4 u_ProjectorMat;

const int MAX_BONES = 150;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

void main() {

    mat4 bone_transform = finalBonesMatrices[a_BoneIds[0]] * a_Weights[0];
		bone_transform += finalBonesMatrices[a_BoneIds[1]] * a_Weights[1];
		bone_transform += finalBonesMatrices[a_BoneIds[2]] * a_Weights[2];
		bone_transform += finalBonesMatrices[a_BoneIds[3]] * a_Weights[3];
			
	vec4 boned_position = bone_transform * vec4(a_Pos, 1.0);

	vs_out.TexCoords = a_TexCoords;
	vs_out.Normal = normalize(u_NormalMat * a_Normal);
	vs_out.Pos = (u_View * u_Model * boned_position).xyz;
	vs_out.ProjTexCoord = u_ProjectorMat * u_Model * boned_position;

	gl_Position = u_Proj * u_View * u_Model * boned_position;
}
