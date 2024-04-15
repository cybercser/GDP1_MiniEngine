#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 9) out;

in VS_OUT {
    vec3 Pos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 ProjTexCoord;
} gs_in[];

out GS_OUT {
    vec3 Pos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 ProjTexCoord;
} gs_out;

void main() {
    for (int i = 0; i < 9; ++i) {
        gl_Position = gl_in[i].gl_Position;
        gs_out.Pos = gs_in[i].Pos;
        gs_out.Normal = gs_in[i].Normal;
        gs_out.TexCoords = gs_in[i].TexCoords;
        gs_out.ProjTexCoord = gs_in[i].ProjTexCoord;

        EmitVertex();
    }
    EndPrimitive();
}