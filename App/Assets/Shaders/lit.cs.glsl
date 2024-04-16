#version 460 core

// Define the buffer to store the output color
layout (binding = 0, rgba32f) uniform image2D outputImage;

// Uniforms for lighting parameters
uniform DirectionalLight u_DirLight;
uniform PointLight u_PointLights[NUM_LIGHTS];
uniform SpotLight u_SpotLights[NUM_LIGHTS];
uniform Material u_Material;
uniform bool u_UseLights;
uniform bool u_UsePointLights;
uniform bool u_UseSpotLights;
uniform bool u_SetLit;
uniform int u_NumPointLights;

// Define reusable variables for texture lookups
vec4 diffuseTextureColor = texture(u_Material.texture_diffuse1, fs_in.TexCoords);

// Blinn-Phong shading, the directional light contribution
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
        specular = lightColor * u_Material.s * pow(max(dot(h, n), 0.0), u_Material.shininess);
    }

    return ambient + diffuse + specular;
}

// Blinn-Phong shading, the point light contribution
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
        specular = lightColor * u_Material.s * pow(max(dot(h, n), 0.0), u_Material.shininess);
    }

    return ambient + diffuse + specular;
}

// Blinn-Phong shading, the spot light contribution
vec3 shadingSpotLight(SpotLight light, vec3 pos, vec3 n) {
    vec3 s = normalize(light.pos - pos);  // s is the light source direction

    float cosTheta = dot(s, normalize(-light.dir));
    float theta = acos(cosTheta);  // theta is the angle between s and the spotlight direction
    float epsilon = light.outerCutoff - light.cutoff;
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
        specular = light.specular * u_Material.s * pow(max(dot(h, n), 0.0), u_Material.shininess);
    }

    return ambient + diffuse + specular;
}

void main() {
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

    vec3 fragPos = ...; // Calculate fragment position in view space
    vec3 fragNormal = ...; // Calculate fragment normal in view space

    vec4 finalColor;

    if (u_UseLights) {
        vec3 lightColor = vec3(0.0);

        // Only one directional light
        vec3 dirColor = shadingDirectionalLight(u_DirLight, fragPos, normalize(fragNormal));

        // Shading for the point lights
        if (u_UsePointLights) {
            for (int i = 0; i < u_NumPointLights; i++) {
                lightColor += shadingPointLight(u_PointLights[i], fragPos, normalize(fragNormal));
            }
        }

        // Shading for the spot lights
        if (u_UseSpotLights) {
            for (int i = 0; i < NUM_LIGHTS; i++) {
                lightColor += shadingSpotLight(u_SpotLights[i], fragPos, normalize(fragNormal));
            }
        }

        finalColor = vec4(dirColor + lightColor, 1.0);
    } else {
        finalColor = vec4(diffuseTextureColor.rgb, 1.0);
    }

    if (u_SetLit) {
        finalColor.rgb *= 1.35;
    }

    // Store the final color in the output image buffer
    imageStore(outputImage, pixelCoords, finalColor);
}