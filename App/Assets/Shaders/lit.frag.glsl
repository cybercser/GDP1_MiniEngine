#version 460 core

in VS_OUT {
    vec3 Pos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 ProjTexCoord;
    vec4 Weights;
} fs_in;

flat in ivec4 BoneIDs;

out vec4 o_FragColor;

struct DirectionalLight {
    vec3 dir;  // Light direction in view space.
    vec4 color;
    float intensity;
};

struct PointLight {
    vec3 pos;  // Light position in view space.

    vec4 color;
    float intensity;

    float c;  // constant attenuation exponent
    float l;  // linear attenuation exponent
    float q;  // quadratic attenuation exponent
};

struct SpotLight {
    vec3 pos;  // Light position in view space.
    vec3 dir;  // Direction of the spotlight in view space.

    float cutoff;       // Cutoff angle ([0, pi/2])
    float outerCutoff;  // Outer cutoff angle ([0, pi/2])

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float c;  // constant attenuation exponent
    float l;  // linear attenuation exponent
    float q;  // quadratic attenuation exponent
};

struct Material {
    sampler2D texture_diffuse1;
    // sampler2D texture_specular1; // specular map
    sampler2D texture_normal1;
    sampler2D texture_height1;
    sampler2D texture_opacity1;
    vec3 s;           // Specular reflectivity
    float shininess;  // Specular shininess factor
};

uniform DirectionalLight u_DirLight;
uniform PointLight u_PointLights[4];
uniform SpotLight u_SpotLights[4];
uniform Material u_Material;
layout(binding = 4) uniform sampler2D u_ProjectorTex;
uniform bool u_UsePointLights;
uniform bool u_UseSpotLights;
uniform bool u_UseProjTex;
uniform bool u_HasBones;

// Blinn-Phong shading, the directional light contribution
// pos is the fragment's position in view space
// n is the fragment's normal in view space
vec3 shadingDirectionalLight(DirectionalLight light, vec3 pos, vec3 n) {
    vec3 lightColor = light.color.rgb * light.intensity;
    // ambient component
    vec3 ambient = lightColor * texture(u_Material.texture_diffuse1, fs_in.TexCoords).rgb;

    // diffuse component
    vec3 s = normalize(-light.dir);  // s is the light source direction
    float sDotN = max(dot(n, s), 0.0);
    vec3 diffuse = lightColor * sDotN * texture(u_Material.texture_diffuse1, fs_in.TexCoords).rgb;

    // specular component
    vec3 specular = vec3(0.0);
    if(sDotN > 0.0) {
        vec3 v = normalize(-pos);  // v is the view direction
        vec3 h = normalize(v + s);
        // vec3 specMap = texture(u_Material.texture_specular1, TexCoord).rgb;
        // specular = light.s * pow(max(dot(h, n), 0.0), u_Material.shininess) * specMap;
        specular = lightColor * u_Material.s * pow(max(dot(h, n), 0.0), u_Material.shininess);
    }

    return ambient + diffuse + specular;
}

// Blinn-Phong shading, the point light contribution
// pos is the fragment's position in view space
// n is the fragment's normal in view space
vec3 shadingPointLight(PointLight light, vec3 pos, vec3 n) {
    vec3 lightColor = light.color.rgb * light.intensity;
    // ambient component
    vec3 ambient = lightColor * texture(u_Material.texture_diffuse1, fs_in.TexCoords).rgb;

    // diffuse component
    vec3 s = normalize(light.pos - pos);  // s is the light source direction
    float sDotN = max(dot(n, s), 0.0);
    vec3 diffuse = lightColor * sDotN * texture(u_Material.texture_diffuse1, fs_in.TexCoords).rgb;

    // specular component
    vec3 specular = vec3(0.0);
    if(sDotN > 0.0) {
        vec3 v = normalize(-pos);  // v is the view direction
        vec3 h = normalize(v + s);
        // vec3 specMap = texture(u_Material.texture_specular1, TexCoord).rgb;
        // specular = light.s * pow(max(dot(h, n), 0.0), u_Material.shininess) * specMap;
        specular = lightColor * u_Material.s * pow(max(dot(h, n), 0.0), u_Material.shininess);
    }

    // attenuation
    float dist = length(light.pos - pos);
    float attenuation = 1.0 / (light.c + light.l * dist + light.q * (dist * dist));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}

// Blinn-Phong shading, the spot light contribution
// pos is the fragment's position in view space
// n is the fragment's normal in view space
vec3 shadingSpotLight(SpotLight light, vec3 pos, vec3 n) {
    vec3 s = normalize(light.pos - pos);  // s is the light source direction

    float cosTheta = dot(s, normalize(-light.dir));
    float theta = acos(cosTheta);  // theta is the angle between s and the spotlight direction
    float epsilon = light.outerCutoff - light.cutoff;
    // float intensity = clamp((light.outerCutoff - theta) / epsilon, 0.0, 1.0);
    float intensity = smoothstep(0.0, 1.0, (light.outerCutoff - theta) / epsilon);

    // ambient component
    vec3 ambient = light.ambient * texture(u_Material.texture_diffuse1, fs_in.TexCoords).rgb;

    // diffuse component
    float sDotN = max(dot(n, s), 0.0);
    vec3 diffuse = light.diffuse * sDotN * texture(u_Material.texture_diffuse1, fs_in.TexCoords).rgb;

    // specular component
    vec3 specular = vec3(0.0);
    if(sDotN > 0.0) {
        vec3 v = normalize(-pos);  // v is the view direction
        vec3 h = normalize(v + s);
        // vec3 specMap = texture(u_Material.texture_specular1, fs_in.TexCoords).rgb;
        // specular = light.s * pow(max(dot(h, n), 0.0), u_Material.shininess) * specMap;
        specular = light.specular * u_Material.s * pow(max(dot(h, n), 0.0), u_Material.shininess);
    }

    // spotlight cookie (projective texture)
    vec3 projTexColor = vec3(0.0);
    if(u_UseProjTex && fs_in.ProjTexCoord.z > 0.0) {
        projTexColor = textureProj(u_ProjectorTex, fs_in.ProjTexCoord).rgb;
    }

    // attenuation
    float dist = length(light.pos - pos);
    float attenuation = 1.0 / (light.c + light.l * dist + light.q * (dist * dist));

    projTexColor *= attenuation * intensity;

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return ambient + diffuse + specular + projTexColor * 0.5;
}

void main() {
    // discard the fragment if its opacity is 0
    if(texture(u_Material.texture_opacity1, fs_in.TexCoords).a < 0.1)
        discard;

    // Only one directional light
    vec3 dirColor = shadingDirectionalLight(u_DirLight, fs_in.Pos, normalize(fs_in.Normal));

    // Shading for the point lights
    vec3 pointColor = vec3(0.0);
    if(u_UsePointLights) {
        for(int i = 0; i < 4; i++) {
            pointColor += shadingPointLight(u_PointLights[i], fs_in.Pos, normalize(fs_in.Normal));
        }
    }

    // Shading for the spot lights
    vec3 spotColor = vec3(0.0);
    if(u_UseSpotLights) {
        for(int i = 0; i < 4; i++) {
            spotColor += shadingSpotLight(u_SpotLights[i], fs_in.Pos, normalize(fs_in.Normal));
        }
    }

    o_FragColor = vec4(dirColor + pointColor + spotColor, 1.0);
}
