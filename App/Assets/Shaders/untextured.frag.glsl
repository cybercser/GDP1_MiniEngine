#version 460 core

in VS_OUT{
	vec3 Pos;
	vec3 Normal;
} fs_in;

out vec4 o_FragColor;

struct DirectionalLight {
	vec3 dir; // Light direction in view space.
	vec3 a;  // Ambient color
	vec3 d;  // Diffuse color
	vec3 s;  // Specular color
};

struct Material {
	vec3 a;
	vec3 d;
	vec3 s;             // Specular reflectivity
	float shininess;    // Specular shininess factor
};

uniform DirectionalLight u_DirLight;
uniform Material u_Material;

// Blinn-Phong shading, the directional light contribution
// pos is the fragment's position in view space
// n is the fragment's normal in view space
vec3 shadingDirectionalLight(DirectionalLight light, vec3 pos, vec3 n) {
	// ambient component
	vec3 ambient = light.a * u_Material.a;

	// diffuse component
	vec3 s = normalize(-light.dir); // s is the light source direction
	float sDotN = max(dot(n, s), 0.0);
	vec3 diffuse = light.d * sDotN * u_Material.d;

	// specular component
	vec3 specular = vec3(0.0);
	if (sDotN > 0.0) {
		vec3 v = normalize(-pos); // v is the view direction
		vec3 h = normalize(v + s);
		// vec3 specMap = texture(u_Material.texture_specular1, TexCoord).rgb;
		// specular = light.s * pow(max(dot(h, n), 0.0), u_Material.shininess) * specMap;
		specular = light.s * u_Material.s * pow(max(dot(h, n), 0.0), u_Material.shininess);
	}

	return ambient + diffuse + specular;
}

void main() {
	vec3 dirColor = shadingDirectionalLight(u_DirLight, fs_in.Pos, normalize(fs_in.Normal));
	o_FragColor = vec4(dirColor, 1.0);
}
