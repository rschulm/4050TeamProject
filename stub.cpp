/******************************************************************************|
| CPSC 4050/6050 Computer Garphics Assignment 5, Daljit Singh Dhillon, 2020    |
| Reference:                                                                   |
|                                                                              |
| Some OpenGL setup code here including math_funcs, and gl_utils               |
| are from Angton Gerdelan and "Anton's OpenGL 4 Tutorials."                   |
| http://antongerdelan.net/opengl/                                             |
| Email: anton at antongerdelan dot net                                        |
| Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland.                |
|******************************************************************************/
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <assert.h>

#include <vector>
#include <iostream>

#include <math.h>
#include <time.h>

#include "maths_funcs.h"   // Anton's maths functions.
#include "gl_utils.h"      // Anton's opengl functions and small utilities like logs
#include "stb_image.h"     // Sean Barrett's image loader with Anton's load_texture()

#include <glm/glm.hpp> // New 
#include <glm/gtc/matrix_transform.hpp> // new
#include <glm/gtc/type_ptr.hpp>



#define _USE_MATH_DEFINES
#define ONE_DEG_IN_RAD (2.0 * M_PI) / 360.0 // 0.017444444

mat4 view_mat;
mat4 proj_mat;
mat4 model_mat;


struct Point {
    GLfloat x, y, z;

	// Conversion function from Point to glm::vec3
   operator glm::vec3() const {
        return glm::vec3(x, y, z);
    }
};

struct Quad {
	Point topLeft, topRight, bottomLeft, bottomRight;
};

struct Triangle {
	Point vertexOne, vertexTwo, vertexThree;

	// Conversion function from Triangle to glm::vec3
    /*operator glm::vec3() const {
        // For the centroid of the triangle
        return (vertexOne + vertexTwo + vertexThree) / 3.0f;
    }*/
	
    // Calculate the vector from vertexOne to vertexTwo
    glm::vec3 vectorOneToTwo() const {
        return glm::vec3(vertexTwo.x - vertexOne.x, vertexTwo.y - vertexOne.y, vertexTwo.z - vertexOne.z);
    }

    // Calculate the vector from vertexOne to vertexThree
    glm::vec3 vectorOneToThree() const {
        return glm::vec3(vertexThree.x - vertexOne.x, vertexThree.y - vertexOne.y, vertexThree.z - vertexOne.z);
    }
};


int pointCount;

void generateHalfCircle(std::vector<float>& coordinates, int numStepsY, int radius, int numStepsTheta) {
    pointCount = numStepsY * numStepsTheta; // Total number of points now

    // Clear the vector to ensure it's empty before adding new values
    coordinates.clear();

    float angleStep = M_PI / static_cast<float>(numStepsY);

	for (int i = 0; i < numStepsY; i++) {
		for(int currAngle = 0; currAngle < numStepsTheta; currAngle++) {
			float angle = M_PI/2.0 - static_cast<float>(i) * angleStep;
			
			float xPos = cos(currAngle * numStepsTheta) * (radius * sin(i * angleStep));
			float yPos = sin(angle) * radius;
			float zPos = (radius * sin(i * angleStep)) * sin(currAngle * numStepsTheta);

			coordinates.push_back(xPos);
			coordinates.push_back(yPos);
			coordinates.push_back(zPos);
			//coordinates.push_back(radius * sin(numStepsTheta));  
		}
	}

    /*for (int i = 0; i < coordinates.size(); i++) {
        std::cout << coordinates[i] << " ";
		if ((i-2) % 3 == 0 ) {
			std::cout << std::endl;
		}
    }*/
}

void calculateNormals(const std::vector<Triangle>& triangles, std::vector<glm::vec3>& normals) {
    for (const auto& triangle : triangles) {
        glm::vec3 normal = glm::normalize(glm::cross(
            triangle.vectorOneToTwo(),  
            triangle.vectorOneToThree()  
        ));

        normals.push_back(normal); // Vertex One
        normals.push_back(normal); // Vertex Two
        normals.push_back(normal); // Vertex Three
    }
}




void calculateTextureCoordinates(const GLfloat* vertexList, std::vector<glm::vec2>& texCoordsVector, int numStepsTheta, int radius) {
    texCoordsVector.clear();

    for (int i = 0; i < pointCount / 3; i++) {
        float x = vertexList[i * 3];
        float z = vertexList[i * 3 + 2];

        // Calculate polar coordinates
        float theta = atan2(z, x) + M_PI; // Map to [0, 2*pi]
        float phi = asin(z / radius) + M_PI / 2.0; // Map to [0, pi]

        // Map polar coordinates to texture coordinates
        float u = theta / (2.0 * M_PI);
        float v = phi / M_PI;

        texCoordsVector.push_back(glm::vec2(u, v));
    }
}


void loadSurfaceOfRevolution() 
{

    int numStepsY, numStepsTheta;

    // Prompt the user for the number of steps along the y dimension
    std::cout << "Enter the number of steps along the y dimension: ";
    std::cin >> numStepsY;

    // Prompt the user for the number of steps along the theta dimension
    std::cout << "Enter the number of steps along the theta dimension: ";
    std::cin >> numStepsTheta;


/*------------------------------CREATE GEOMETRY-------------------------------*/
	
	std::vector<float> coordinates;

	int radius = 1;
	generateHalfCircle(coordinates, numStepsY, radius, numStepsTheta);


	// Trying to generate the P[i][j] array of points based on 'coordinates' vector
	Point pointArray[numStepsY][numStepsTheta];

	for (int i = 0; i < numStepsY; i++) {
		for (int j = 0; j < numStepsTheta; j++) {
			Point currPoint;
			currPoint.x = coordinates[(numStepsTheta * i + j) * 3];     // set x
			currPoint.y = coordinates[(numStepsTheta * i + j) * 3 + 1]; // set y
			currPoint.z = coordinates[(numStepsTheta * i + j) * 3 + 2]; // set z

			pointArray[i][j] = currPoint;
			//std::cout << "Point = (" << currPoint.x << ", " << currPoint.y << ", " << currPoint.z << ")" << std::endl;
		}
	}

	// Making quads from P[i][j] array
	std::vector<Quad> quadList;
	for (int i = 0; i < numStepsY - 1; i++) {
		for (int j = 0; j < numStepsTheta - 1; j++) {
			Quad currQuad;
			currQuad.topLeft = pointArray[i][j];
			currQuad.topRight = pointArray[i][j + 1];
			currQuad.bottomLeft = pointArray[i + 1][j];
			currQuad.bottomRight = pointArray[i + 1][j + 1];

			quadList.push_back(currQuad);
		}
		// This is for the quad that is part of index = 0 and index = max to loop back
		Quad currQuad;
		currQuad.topLeft = pointArray[i][numStepsTheta - 1];
		currQuad.topRight = pointArray[i][0];
		currQuad.bottomLeft = pointArray[i + 1][numStepsTheta - 1];
		currQuad.bottomRight = pointArray[i + 1][0];

		quadList.push_back(currQuad);
	}

	// Making triangles from quadList
	std::vector<Triangle> triangleList;
	for (const auto& quad : quadList) {
		Triangle currTriangle;

		// First triangle = first half of quad
		currTriangle.vertexOne = quad.topLeft;
		currTriangle.vertexTwo = quad.bottomRight;
		currTriangle.vertexThree = quad.bottomLeft;
		triangleList.push_back(currTriangle);

		// Second triangle = second half of quad
		currTriangle.vertexOne = quad.topLeft;
		currTriangle.vertexTwo = quad.topRight;
		currTriangle.vertexThree = quad.bottomRight;
		triangleList.push_back(currTriangle);
	}

	GLfloat vertexList[triangleList.size() * 9]; // 3 vertices per triangle, each with 3 coordinates
	for (int i = 0; i < triangleList.size(); i++) {
		// First vertex
		vertexList[i * 9] = triangleList[i].vertexOne.x;
		vertexList[i * 9 + 1] = triangleList[i].vertexOne.y;
		vertexList[i * 9 + 2] = triangleList[i].vertexOne.z;

		// Second vertex
		vertexList[i * 9 + 3] = triangleList[i].vertexTwo.x;
		vertexList[i * 9 + 4] = triangleList[i].vertexTwo.y;
		vertexList[i * 9 + 5] = triangleList[i].vertexTwo.z;

		// Third vertex
		vertexList[i * 9 + 6] = triangleList[i].vertexThree.x;
		vertexList[i * 9 + 7] = triangleList[i].vertexThree.y;
		vertexList[i * 9 + 8] = triangleList[i].vertexThree.z;
	}

	pointCount = triangleList.size() * 9; // Set the correct amount of points for drawSurfaceOfRevolution() later
	
	
	// VAO -- vertex attribute objects bundle the various things associated with vertices
	GLuint vao;
	glGenVertexArrays (1, &vao);   // generating and binding is common pattern in OpenGL
	glBindVertexArray (vao);       // basically setting up memory and associating it

	// VBO -- vertex buffer object to contain coordinates
	// MODIFY THE FOLLOWING BLOCK OF CODE APPRORIATELY FOR YOUR SURFACE OF REVOLUTION
	GLuint points_vbo;
	glGenBuffers(1, &points_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glBufferData(GL_ARRAY_BUFFER, pointCount * sizeof (GLfloat), vertexList, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	// VBO -- normals -- needed for shading calcuations
	// ADD CODE TO POPULATE AND LOAD PER-VERTEX SURFACE NORMALS  
	// [HINT] Vertex normals are organized in same order as that for vertex coordinates

	std::vector<glm::vec3> normalsVector;
	calculateNormals(triangleList, normalsVector);

	GLuint normals_vbo;
	glGenBuffers(1, &normals_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
	glBufferData(GL_ARRAY_BUFFER, normalsVector.size() * sizeof(glm::vec3), &normalsVector[0], GL_STATIC_DRAW);
	// Assuming attribute location 1 for normals
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);


    // VBO -- vt -- texture coordinates
	// ADD CODE TO POPULATE AND LOAD PER-VERTEX TEXTURE COORDINATES  
	// [HINT] texture coordinates are organized in same order as that for vertex coordinates
	// [HINT] there are two texture coordinates instead of three vertex coordinates for each vertex
	
	std::vector<glm::vec2> texCoordsVector;
	calculateTextureCoordinates(vertexList, texCoordsVector, numStepsTheta, radius);


	GLuint texCoord_vbo;
	glGenBuffers(1, &texCoord_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, texCoord_vbo);
	glBufferData(GL_ARRAY_BUFFER, texCoordsVector.size() * sizeof(float), &texCoordsVector[0], GL_STATIC_DRAW);
	
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
}


glm::vec3 lightPosition(-2.0f, -1.0f, 0.0f);
GLuint s_p;
float specularExponent = 32.0f;
bool enableDiffuse = true;
bool enableSpecular = true;
bool enableTexture = true;



void loadUniforms(GLuint shader_programme)
{	

	s_p = shader_programme;
/*---------------------------SET RENDERING DEFAULTS---------------------------*/

	// Choose vertex and fragment shaders to use as well as view and proj matrices.
	int model_mat_location = glGetUniformLocation (shader_programme, "model_mat");
	int view_mat_location  = glGetUniformLocation (shader_programme, "view_mat");
	int proj_mat_location  = glGetUniformLocation (shader_programme, "proj_mat");
	
	glUniformMatrix4fv (view_mat_location, 1, GL_FALSE, view_mat.m);
	glUniformMatrix4fv (proj_mat_location, 1, GL_FALSE, proj_mat.m);
	glUniformMatrix4fv (model_mat_location, 1, GL_FALSE, model_mat.m);
	
	// WRITE CODE TO LOAD OTHER UNIFORM VARIABLES LIKE FLAGS FOR ENABLING OR DISABLING CERTAIN FUNCTIONALITIES

	// Load light-related uniforms
	//glm::vec3 lightPosition(-2.0f, -1.0f, 0.0f);
    int lightPosLocation = glGetUniformLocation(shader_programme, "lightPos");
    glUniform3f(lightPosLocation, lightPosition.x, lightPosition.y, lightPosition.z);

    int enableDiffuseLocation = glGetUniformLocation(s_p, "enableDiffuse");
    glUniform1i(enableDiffuseLocation, enableDiffuse);

    int enableSpecularLocation = glGetUniformLocation(s_p, "enableSpecular");
    glUniform1i(enableSpecularLocation, enableSpecular);

    int enableTextureLocation = glGetUniformLocation(s_p, "enableTexture");
    glUniform1i(enableTextureLocation, enableTexture);
	
}

void drawSurfaceOfRevolution()
{
	// MODIFY THIS LINE OF CODE APPRORIATELY FOR YOUR SURFACE OF REVOLUTION
	//glDrawArrays(GL_POINTS, 0, pointCount);
	glDrawArrays(GL_TRIANGLES, 0, pointCount);
}






//glm::vec3 lightPosition(-2.0f, -1.0f, 0.0f);
/*float specularExponent = 32.0f;
bool enableDiffuse = true;
bool enableSpecular = true;
bool enableTexture = true;*/

	
void keyboardFunction(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// MODIFY THIS FUNCTION FOR KEYBOARD INTERACTIVITY
	//GLFW Reference Links:
	// Callback Example: https://www.glfw.org/docs/3.3/input_guide.html#input_key
	// List of Keys: https://www.glfw.org/docs/3.3/group__keys.html
	
    /*if (key == GLFW_KEY_E && action == GLFW_PRESS)
    {
		printf("\nKey 'E' pressed.... \n");
        // Example case. Key 'E' pressed. Doing nothing
	}
        
	if (GLFW_PRESS == glfwGetKey (g_window, GLFW_KEY_ESCAPE)) {
		// Close window when esacape is pressed
			glfwSetWindowShouldClose (g_window, 1);
	}*/


	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
            case GLFW_KEY_E:
                // Toggle diffuse shading effects
                enableDiffuse = !enableDiffuse;
                printf("Diffuse Shading: %s\n", enableDiffuse ? "Enabled" : "Disabled");
                break;
            case GLFW_KEY_S:
                // Toggle specular lighting effects
                enableSpecular = !enableSpecular;
                printf("Specular Lighting: %s\n", enableSpecular ? "Enabled" : "Disabled");
                break;
            case GLFW_KEY_T:
                // Toggle use of texture as diffuse albedo
                enableTexture = !enableTexture;
                printf("Texture Mapping: %s\n", enableTexture ? "Enabled" : "Disabled");
                break;
            case GLFW_KEY_UP:
                // Move light position up
                lightPosition.y -= 0.1f;
                printf("Light Position: (%.2f, %.2f, %.2f)\n", lightPosition.x, lightPosition.y, lightPosition.z);
                break;
            case GLFW_KEY_DOWN:
                // Move light position down
                lightPosition.y += 0.1f;
                printf("Light Position: (%.2f, %.2f, %.2f)\n", lightPosition.x, lightPosition.y, lightPosition.z);
                break;
            case GLFW_KEY_LEFT:
                // Move light position left
                lightPosition.x += 0.1f;
                printf("Light Position: (%.2f, %.2f, %.2f)\n", lightPosition.x, lightPosition.y, lightPosition.z);
                break;
            case GLFW_KEY_RIGHT:
                // Move light position right
                lightPosition.x -= 0.1f;
                printf("Light Position: (%.2f, %.2f, %.2f)\n", lightPosition.x, lightPosition.y, lightPosition.z);
                break;
            case GLFW_KEY_U:
                // Increase specular exponent
                specularExponent += 5.0f;
                printf("Specular Exponent: %.2f\n", specularExponent);
                break;
            case GLFW_KEY_D:
                // Decrease specular exponent
                specularExponent -= 5.0f;
                printf("Specular Exponent: %.2f\n", specularExponent);
                break;
        }
    }
    
    // Update uniforms in the shader program
    int lightPosLocation = glGetUniformLocation(s_p, "lightPos");
    glUniform3f(lightPosLocation, lightPosition.x, lightPosition.y, lightPosition.z);

    int specExpLocation = glGetUniformLocation(s_p, "specularExponent");
    glUniform1f(specExpLocation, specularExponent);

    int enableDiffuseLocation = glGetUniformLocation(s_p, "enableDiffuse");
    glUniform1i(enableDiffuseLocation, enableDiffuse);

    int enableSpecularLocation = glGetUniformLocation(s_p, "enableSpecular");
    glUniform1i(enableSpecularLocation, enableSpecular);

    int enableTextureLocation = glGetUniformLocation(s_p, "enableTexture");
    glUniform1i(enableTextureLocation, enableTexture);

}