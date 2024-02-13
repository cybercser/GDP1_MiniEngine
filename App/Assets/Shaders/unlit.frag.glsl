#version 460 core

in VS_OUT{
	vec2 TexCoords;
} fs_in;

out vec4 o_FragColor;

uniform sampler2D texture_diffuse1;

void main() {
	o_FragColor = texture(texture_diffuse1, fs_in.TexCoords);
}
