#version 460 core
out vec4 o_FragColor;

in vec3 Normal;
in vec3 Pos;

uniform vec3 u_CameraPos;
uniform samplerCube u_Skybox;

void main()
{
    vec3 I = normalize(Pos - u_CameraPos);
    vec3 R = reflect(I, normalize(Normal));
    o_FragColor = vec4(texture(u_Skybox, R).rgb, 1.0);
}
