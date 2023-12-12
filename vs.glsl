#version 410

layout (location = 0) in vec3 vtxPosition;
layout (location = 1) in vec3 vtxNormal;
layout (location = 2) in vec2 texCoords;

uniform mat4 proj_mat, view_mat, model_mat;

out vec2 texture_coords;
out vec3 frag_Position;
out vec3 normal;

void main () {
	gl_Position = proj_mat * view_mat * model_mat * vec4(vtxPosition, 1.0);

	texture_coords = texCoords;
	frag_Position = vtxPosition;
	normal = normalize(mat3(transpose(inverse(model_mat))) * vtxNormal);
}
