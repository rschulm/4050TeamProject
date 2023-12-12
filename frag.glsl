#version 410

// Define INPUTS from fragment shader
uniform mat4 view_mat;

smooth in vec3 lightContrib;
in vec2 texture_coords;
in vec3 frag_Position;
in vec3 normal;

// And from the uniform outputs for the textures setup in main.cpp.
uniform sampler2D texture00;
uniform sampler2D texture01;
uniform vec3 light_position;
uniform vec3 light_color;
uniform float shininess;
uniform bool useText;
uniform bool useSpec;
uniform bool useDiff;

out vec4 fragment_color; //RGBA color
void main () {
    float toonLevels = 4.0;
    float toonThreshold = 1.0 / toonLevels;
    vec3 lightDir = normalize(light_position - frag_Position);
	vec3 viewDir = normalize(vec3(0.0, 0.0, 10.0) - frag_Position);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float ambientStrength = 0.2;

    float specFactor = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    float specToonFactor = floor(specFactor * toonLevels) / toonLevels;
	vec3 specular = light_color * specToonFactor;

    float diffFactor = max(dot(normal, lightDir), 0.0);
    float diffToonFactor = floor(diffFactor * toonLevels) / toonLevels;
    vec3 diffuse = light_color * diffToonFactor;

	vec3 ambient = light_color * ambientStrength;

	vec3 lightContrib = ambient;
    if(useSpec) {lightContrib += specular;}
    if(useDiff) {lightContrib += diffuse;}
    if(useText) {
        fragment_color = texture2D(texture00, texture_coords)  * vec4(lightContrib, 1.0);
    }
    else{
        fragment_color = vec4(1.0, 0.5, 0.0, 1.0) * vec4(lightContrib, 1.0);
    }
}
