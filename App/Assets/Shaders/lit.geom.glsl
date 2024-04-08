#version 460 core

layout (points) in;
layout (triangle_strip, max_vertices = 3) out;

in G_VS_OUT {
    vec3 Pos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 ProjTexCoord;
    vec4 Weights;
} gs_in[];

out VS_OUT{
    vec3 Pos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 ProjTexCoord;
    vec4 Weights;
} vs_out;

void main() {
    for (int i = 0; i < 3; ++i) {
        vs_out.Pos = gs_in[0].Pos;
        vs_out.Normal = gs_in[0].Normal;
        vs_out.TexCoords = gs_in[0].TexCoords;
        vs_out.ProjTexCoord = gs_in[0].ProjTexCoord;
        vs_out.Weights = gs_in[0].Weights;
        gl_Position = gl_in[0].gl_Position;
        EmitVertex();
    }
	
    EndPrimitive();
}
