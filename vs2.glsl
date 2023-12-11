#version 410

layout (location = 0) in vec3 vtxPosition;
layout (location = 1) in vec3 vtxNormal;    // New attribute for normals
layout (location = 2) in vec2 vtxTexCoord;  // New attribute for texture coordinates

uniform mat4 proj_mat, view_mat, model_mat;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec3 LightPosInView;

void main () {
    gl_Position = proj_mat * view_mat * model_mat * vec4(vtxPosition, 1.0);

    // Pass attributes to the fragment shader
    FragPos = vec3(model_mat * vec4(vtxPosition, 1.0));
    Normal = normalize(mat3(transpose(inverse(model_mat))) * vtxNormal); // Transform normal to world space
    TexCoords = vtxTexCoord;
	LightPosInView = vec3(view_mat * model_mat * vec4(vtxPosition, 1.0));

}

