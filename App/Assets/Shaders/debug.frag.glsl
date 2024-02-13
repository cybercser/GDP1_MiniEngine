#version 460 core

uniform vec4 u_Color;

out vec4 o_FragColor;

void main()
{
    o_FragColor = u_Color;
}
