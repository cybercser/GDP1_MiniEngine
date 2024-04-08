#version 460 core

in VS_OUT {
    vec3 Pos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 ProjTexCoord;
} fs_in;

out vec4 o_FragColor;

struct Material {
    sampler2D texture_diffuse1;
    // sampler2D texture_specular1; // specular map
    sampler2D texture_normal1;
    sampler2D texture_height1;
    sampler2D texture_opacity1;
    vec3 s;           // Specular reflectivity
    float shininess;  // Specular shininess factor
};

uniform Material u_Material;

void main() {
    // discard the fragment if its opacity is 0
    if(texture(u_Material.texture_opacity1, fs_in.TexCoords).a < 0.1)
        discard;

    vec4 finalColor = texture(u_Material.texture_diffuse1, fs_in.TexCoords);

    o_FragColor = finalColor;
}
