#version 460 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec3 a_Normal;

out vec3 Normal;
out vec3 Pos;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;

void main()
{
    Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
    Pos = vec3(u_Model * vec4(a_Pos, 1.0));
    gl_Position = u_Proj * u_View * u_Model * vec4(a_Pos, 1.0);
}
