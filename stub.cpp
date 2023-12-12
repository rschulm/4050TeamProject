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
#include <iostream>
#include <vector>


#include <math.h>
#include <time.h>

#include "maths_funcs.h"   // Anton's maths functions.
#include "gl_utils.h"      // Anton's opengl functions and small utilities like logs
#include "stb_image.h"     // Sean Barrett's image loader with Anton's load_texture()

#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>

#define _USE_MATH_DEFINES
#define ONE_DEG_IN_RAD (2.0 * M_PI) / 360.0 // 0.017444444


mat4 view_mat;
mat4 proj_mat;
mat4 model_mat;


int pointCount;
int basePointCount;


void loadSurfaceOfRevolution() 
{
/*------------------------------CREATE GEOMETRY-------------------------------*/
	int stepsY = 0;
	int stepsT = 0;

	std::cout<<"Enter how many steps along the Y axis: ";
	std::cin>>stepsY;
	while(stepsY < 3){
		std::cout<<"Must be at least 3 steps. Please Re-enter: ";
		std::cin>>stepsY;
	}
	std::cout<<"Enter how many steps around the y axis: ";
	std::cin>>stepsT;
	while(stepsT < 3){
		std::cout<<"Must be at least 3 steps. Please Re-enter: ";
		std::cin>>stepsT;
	}

	//calcluate points along the base curve (circular arc with equation x^2 + y^2 = 1)
	std::vector<float> basePoints;
	for(int i=0; i<=stepsY; i++){
		float y = 1.0 - (i*(2.0/stepsY));
		float x = sqrt(1.0-pow(y, 2));
		//float x = -1*pow(y, 2) + 1;
		//float x = sqrt(y+1);

		basePoints.push_back(x);
		basePoints.push_back(y);
		basePoints.push_back(2);

		//std::cout<< x << ", " << y << "\n";
	}
	basePointCount = basePoints.size();


	//calculate all verticies based on the base curve at each step of theta
	pointCount = stepsY*stepsT;
	
	GLfloat vp[pointCount*18];    //array of vertex points
	GLfloat normals[pointCount*18]; //array of normals
	GLfloat vt[pointCount*12]; //array of texture coordinates

	//for each level of Y, starting from the top, iterate through each vertex
	for(int i = 0; i < stepsY; i++){
		for(int j = 0; j < stepsT; j++){
			int vIndex = (i*stepsY*18)+(j*18);
			int nIndex = (i*stepsY*18)+(j*18);
			int tIndex = (i*stepsY*12)+(j*12);

			GLfloat Pij[3] = {basePoints[i*3]*cos(j*(2*M_PI/stepsT)),
							  basePoints[i*3+1],
							  basePoints[i*3]*sin(j*(2*M_PI/stepsT))};
			GLfloat Piij[3] = {basePoints[(i+1)*3]*cos(j*(2*M_PI/stepsT)),
							   basePoints[(i+1)*3+1],
							   basePoints[(i+1)*3]*sin(j*(2*M_PI/stepsT))};
			GLfloat Pijj[3] = {basePoints[i*3]*cos((j+1)*(2*M_PI/stepsT)),
							   basePoints[i*3+1],
							   basePoints[i*3]*sin((j+1)*(2*M_PI/stepsT))};
			GLfloat Piijj[3] = {basePoints[(i+1)*3]*cos((j+1)*(2*M_PI/stepsT)),
							    basePoints[(i+1)*3+1],
								basePoints[(i+1)*3]*sin((j+1)*(2*M_PI/stepsT))};

			//load the points of the two triangles with their top left corner at P i,j
			//P i,j
			vp[vIndex+0] = Pij[0]; //x
			vp[vIndex+1] = Pij[1]; //y
			vp[vIndex+2] = Pij[2]; //z
			
			//P i+1,j+1
			vp[vIndex+3] = Piijj[0]; //x
			vp[vIndex+4] = Piijj[1]; //y
			vp[vIndex+5] = Piijj[2]; //z
			
			//P i,j+1
			vp[vIndex+6] = Pijj[0]; //x
			vp[vIndex+7] = Pijj[1]; //y
			vp[vIndex+8] = Pijj[2]; //z
			
			//P i,j
			vp[vIndex+9] = Pij[0]; //x
			vp[vIndex+10] = Pij[1]; //y
			vp[vIndex+11] = Pij[2]; //z
			
			//P i+1,j
			vp[vIndex+12] = Piij[0]; //x
			vp[vIndex+13] = Piij[1]; //y
			vp[vIndex+14] = Piij[2]; //z
			
			//P i+1,j+1
			vp[vIndex+15] = Piijj[0]; //x
			vp[vIndex+16] = Piijj[1]; //y
			vp[vIndex+17] = Piijj[2]; //z
			

			
			//calculate and load normals
        	glm::vec3 edgeA1 = glm::vec3(Piijj[0] - Pij[0], Piijj[1] - Pij[1], Piijj[2] - Pij[2]);
    		glm::vec3 edgeA2 = glm::vec3(Pijj[0] - Pij[0], Pijj[1] - Pij[1], Pijj[2] - Pij[2]);
			glm::vec3 normalA = glm::normalize(glm::cross(edgeA1, edgeA2));
			for(int i=0; i<3; i++){
    			normals[nIndex+(i*3)+0] = normalA.x;
				normals[nIndex+(i*3)+1] = normalA.y;
				normals[nIndex+(i*3)+2] = normalA.z;
			}
			glm::vec3 edgeB1 = glm::vec3(Piij[0] - Pij[0], Piij[1] - Pij[1], Piij[2] - Pij[2]);
    		glm::vec3 edgeB2 = glm::vec3(Piijj[0] - Pij[0], Piijj[1] - Pij[1], Piijj[2] - Pij[2]);
			glm::vec3 normalB = glm::normalize(glm::cross(edgeB1, edgeB2));
			for(int i=0; i<3; i++){
    			normals[nIndex+(i*3)+9] = normalB.x;
				normals[nIndex+(i*3)+10] = normalB.y;
				normals[nIndex+(i*3)+11] = normalB.z;
			}



			//calculate and load texture coordinates
			vt[tIndex+0] = (j*(2*M_PI/stepsT))/(2*M_PI); //u value (theta)
			vt[tIndex+1] = (Pij[1]+1)/2; //v value

			vt[tIndex+2] = (j+1)*(2*M_PI/stepsT)/(2*M_PI); //u value (theta)
			vt[tIndex+3] = (Piijj[1]+1)/2; //v value

			vt[tIndex+4] = (j+1)*(2*M_PI/stepsT)/(2*M_PI); //u value (theta)
			vt[tIndex+5] = (Pijj[1]+1)/2; //v value

			vt[tIndex+6] = j*(2*M_PI/stepsT)/(2*M_PI); //u value (theta)
			vt[tIndex+7] = (Pij[1]+1)/2; //v value

			vt[tIndex+8] = j*(2*M_PI/stepsT)/(2*M_PI); //u value (theta)
			vt[tIndex+9] = (Piij[1]+1)/2; //v value

			vt[tIndex+10] = (j+1)*(2*M_PI/stepsT)/(2*M_PI); //u value (theta)
			vt[tIndex+11] = (Piijj[1]+1)/2; //v value
		}
	}
	
	
	// VAO -- vertex attribute objects bundle the various things associated with vertices
	GLuint vao;
	glGenVertexArrays (1, &vao);   // generating and binding is common pattern in OpenGL
	glBindVertexArray (vao);       // basically setting up memory and associating it

	// VBO -- vertex buffer object to contain coordinates
	// MODIFY THE FOLLOWING BLOCK OF CODE APPRORIATELY FOR YOUR SURFACE OF REVOLUTION
	GLuint points_vbo;
	glGenBuffers(1, &points_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glBufferData(GL_ARRAY_BUFFER, pointCount*18 * sizeof (GLfloat), vp, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	// VBO -- normals -- needed for shading calcuations
	// ADD CODE TO POPULATE AND LOAD PER-VERTEX SURFACE NORMALS  
	// [HINT] Vertex normals are organized in same order as that for vertex coordinates
	GLuint normals_vbo;
	glGenBuffers(1, &normals_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
	glBufferData(GL_ARRAY_BUFFER, pointCount*18 * sizeof (GLfloat), normals, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);

    // VBO -- vt -- texture coordinates
	// ADD CODE TO POPULATE AND LOAD PER-VERTEX TEXTURE COORDINATES  
	// [HINT] texture coordinates are organized in same order as that for vertex coordinates
	// [HINT] there are two texture coordinates instead of three vertex coordinates for each vertex
	GLuint vt_vbo;
	glGenBuffers(1, &vt_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vt_vbo);
	glBufferData(GL_ARRAY_BUFFER, pointCount*12 * sizeof (GLfloat), vt, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(2);
}

GLfloat lightX = 0.0;
GLfloat lightY = 0.0;
GLfloat lightZ = 10.0;
GLfloat shine = 32.0;
bool useTexture = true;
bool useSpecular = true;
bool useDiffuse = true;

void loadUniforms(GLuint shader_programme)
{	
/*---------------------------SET RENDERING DEFAULTS---------------------------*/

	// Choose vertex and fragment shaders to use as well as view and proj matrices.
	int model_mat_location = glGetUniformLocation (shader_programme, "model_mat");
	int view_mat_location  = glGetUniformLocation (shader_programme, "view_mat");
	int proj_mat_location  = glGetUniformLocation (shader_programme, "proj_mat");
	int light_pos_location  = glGetUniformLocation (shader_programme, "light_position");
	int light_col_location  = glGetUniformLocation (shader_programme, "light_color");
	int shininess_location  = glGetUniformLocation (shader_programme, "shininess");
	int useT_location  = glGetUniformLocation (shader_programme, "useText");
	int useS_location  = glGetUniformLocation (shader_programme, "useSpec");
	int useD_location  = glGetUniformLocation (shader_programme, "useDiff");
	
	glUniformMatrix4fv (view_mat_location, 1, GL_FALSE, view_mat.m);
	glUniformMatrix4fv (proj_mat_location, 1, GL_FALSE, proj_mat.m);
	glUniformMatrix4fv (model_mat_location, 1, GL_FALSE, model_mat.m);
	glUniform3f(light_pos_location, lightX, lightY, lightZ);
	glUniform3f(light_col_location, 1.0, 1.0, 1.0);
	glUniform1f(shininess_location, shine);
	glUniform1f(useT_location, useTexture);
	glUniform1f(useS_location, useSpecular);
	glUniform1f(useD_location, useDiffuse);

	
	// WRITE CODE TO LOAD OTHER UNIFORM VARIABLES LIKE FLAGS FOR ENABLING OR DISABLING CERTAIN FUNCTIONALITIES
}

void drawSurfaceOfRevolution()
{
	// MODIFY THIS LINE OF CODE APPRORIATELY FOR YOUR SURFACE OF REVOLUTION
	glDrawArrays(GL_TRIANGLES, 0, pointCount*18);
}
	
void keyboardFunction(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// MODIFY THIS FUNCTION FOR KEYBOARD INTERACTIVITY
	//GLFW Reference Links:
	// Callback Example: https://www.glfw.org/docs/3.3/input_guide.html#input_key
	// List of Keys: https://www.glfw.org/docs/3.3/group__keys.html
	
    if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
		printf("\nLight Position Moved Up\n");
		lightY -= 1.0;
        // Example case. Key 'E' pressed. Doing nothing
	}

	if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
		printf("\nLight Position Moved Down\n");
		lightY += 1.0;
        // Example case. Key 'E' pressed. Doing nothing
	}

	if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
		printf("\nLight Position Moved Right\n");
		lightX -= 1.0;
        // Example case. Key 'E' pressed. Doing nothing
	}

	if (key == GLFW_KEY_A && action == GLFW_PRESS)
    {
		printf("\nLight Position Moved Left\n");
		lightX += 1.0;
        // Example case. Key 'E' pressed. Doing nothing
	}

	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
    {
		printf("\nLight Position Moved Closer\n");
		lightZ += 1.0;
        // Example case. Key 'E' pressed. Doing nothing
	}

	if (key == GLFW_KEY_E && action == GLFW_PRESS)
    {
		printf("\nLight Position Moved Further\n");
		lightZ -= 1.0;
        // Example case. Key 'E' pressed. Doing nothing
	}

	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
		printf("\nShine Increased\n");
		shine *= 2.0;
        // Example case. Key 'E' pressed. Doing nothing
	}

	if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
		printf("\nShine Decreased\n");
		shine /= 2.0;
        // Example case. Key 'E' pressed. Doing nothing
	}

	if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
		printf("\nTexture Mapping Toggled\n");
		useTexture = !useTexture;
        // Example case. Key 'E' pressed. Doing nothing
	}

	if (key == GLFW_KEY_T && action == GLFW_PRESS)
    {
		printf("Specular Lighting Toggled\n");
		useSpecular = !useSpecular;
        // Example case. Key 'E' pressed. Doing nothing
	}

	if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    {
		printf("\nDiffuse Lighting Toggled\n");
		useDiffuse = !useDiffuse;
        // Example case. Key 'E' pressed. Doing nothing
	}
        
	if (GLFW_PRESS == glfwGetKey (g_window, GLFW_KEY_ESCAPE)) {
		// Close window when esacape is pressed
			glfwSetWindowShouldClose (g_window, 1);
	}

}
