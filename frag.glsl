/*#version 410
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 LightPosInView;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform bool enableTexture = true;
uniform bool enableSpecular = true;
uniform bool enableDiffuse = true;

uniform float specularExponent;

uniform sampler2D texture00;
uniform sampler2D texture01;

out vec4 fragment_color;

void main()
{
    // Ambient lighting
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);

    // Diffuse lighting
    vec3 lightDir = normalize(lightPos - FragPos); 
    float diff = max(dot(Normal, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);

    // Specular lighting (Blinn-Phong)
    float specularStrength = 0.5;
    float shininess = 32.0;

    vec3 viewDir = normalize(viewPos - FragPos); 
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float spec = pow(max(dot(Normal, halfwayDir), 0.0), specularExponent);
    vec3 specular = specularStrength * spec * vec3(1.0, 1.0, 1.0);

    // Sample textures
    vec3 texColor0 = texture(texture00, TexCoords).xyz;
    vec3 texColor1 = texture(texture01, TexCoords).xyz;

    // Debug: Output texture colors
    //fragment_color = vec4(texColor0, 1.0); // Debug check color
    //fragment_color = vec4(1.0,0.5,0.0,1.0);

    // Combine textures with lighting
    vec3 result = (ambient + diffuse + specular) * mix(texColor0, texColor1, 0.5);
    fragment_color = vec4(result, 1.0);
}*/



#version 410

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 LightPosInView;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform bool enableTexture;
uniform bool enableSpecular;
uniform bool enableDiffuse;

uniform float specularExponent;

uniform sampler2D texture00;
uniform sampler2D texture01;

out vec4 fragment_color;

void main()
{
    // Ambient lighting
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);

    // Diffuse lighting
    vec3 lightDir = normalize(lightPos - FragPos); 
    float diff = max(dot(Normal, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);

    // Specular lighting (Blinn-Phong)
    float specularStrength = 0.5;
    float shininess = 32.0;
    vec3 viewDir = normalize(viewPos - FragPos); 
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), specularExponent);
    vec3 specular = specularStrength * spec * vec3(1.0, 1.0, 1.0);

    // Sample textures only if enableTexture is true
    vec3 texColor0 = enableTexture ? texture(texture00, TexCoords).xyz : vec3(1.0);
    vec3 texColor1 = enableTexture ? texture(texture01, TexCoords).xyz : vec3(1.0);

    // Combine textures with lighting only if enableDiffuse or enableSpecular is true
    vec3 result = ambient;
    if (enableDiffuse) {
        result += diffuse;
    }
    if (enableSpecular) {
        result += specular;
    }
    result *= mix(texColor0, texColor1, 0.5);

    fragment_color = vec4(result, 1.0);
}
