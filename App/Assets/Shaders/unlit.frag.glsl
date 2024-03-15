#version 460 core

in VS_OUT{
	vec2 TexCoords;
} fs_in;

out vec4 o_FragColor;

uniform sampler2D texture_diffuse;

void main() {
	o_FragColor = texture(texture_diffuse, fs_in.TexCoords);
}
