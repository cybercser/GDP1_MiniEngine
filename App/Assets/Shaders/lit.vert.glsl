#version 460 core

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangents;
layout(location = 4) in vec3 a_Bitangents;
layout(location = 5) in ivec4 boneIds;     // INT pointer
layout(location = 6) in vec4 weights;

// Instancing Transformations
layout (location = 7) in mat4 instanceMatrix;

flat out ivec4 outBoneIDs;

out VS_OUT{
	vec3 Pos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 ProjTexCoord;
    vec4 Weights;
} vs_out;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;
uniform mat3 u_NormalMat;

uniform mat4 u_ProjectorMat;


const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 bones[MAX_BONES];

void main() {

	vec4 totalPosition = vec4(0.0f);
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(boneIds[i] == -1) 
            continue;
        if(boneIds[i] >= MAX_BONES) 
        {
            totalPosition = vec4(a_Pos,1.0f);
            break;
        }
        vec4 localPosition = bones[boneIds[i]] * vec4(a_Pos,1.0f);
        totalPosition += localPosition * weights[i];
        vec3 localNormal = mat3(bones[boneIds[i]]) * a_Normal;
    }

    if(boneIds[0] == -1) 
        totalPosition = vec4(a_Pos, 1.0);
		
    mat4 viewModel = u_View * u_Model;
    gl_Position =  u_Proj * viewModel * totalPosition;

    
    outBoneIDs = boneIds;

    vs_out.Weights = weights;
	vs_out.TexCoords = a_TexCoords;
	vs_out.Normal = normalize(u_NormalMat * a_Normal);
	vs_out.Pos = (u_View * u_Model * vec4(a_Pos, 1.0)).xyz;
	vs_out.ProjTexCoord = u_ProjectorMat * u_Model * vec4(a_Pos, 1.0);

}
