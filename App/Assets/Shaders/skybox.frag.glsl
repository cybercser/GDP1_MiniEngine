#version 460 core
out vec4 o_FragColor;

in vec3 TexCoords;

uniform samplerCube u_Skybox;

void main()
{
    o_FragColor = texture(u_Skybox, TexCoords);
}