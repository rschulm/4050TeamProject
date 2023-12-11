/******************************************************************************|
| CPSC 4050/6050 Computer Garphics Assignment 5, Daljit Singh Dhillon, 2020    |
| Modified from:                                                               |
|                                                                              |
| DPA 8090 Example Code, Eric Patterson, 2017                                  |
|                                                                              |
| This is mostly plain C but uses a few things from C++ so needs C++ compiler. |
| Some OpenGL setup code here and obj_parser, math_funcs, and gl_utils         |
| are from Angton Gerdelan and "Anton's OpenGL 4 Tutorials."                   |
| http://antongerdelan.net/opengl/                                             |
| Email: anton at antongerdelan dot net                                        |
| Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland.                |
|******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <math.h>
#include <time.h>

#include <GL/glew.h>       // Include GLEW (or new version of GL if on Windows).
#include <GLFW/glfw3.h>    // GLFW helper library.

#include "maths_funcs.h"   // Anton's maths functions.
#include "gl_utils.h"      // Anton's opengl functions and small utilities like logs
#include "obj_parser.h"    // Anton's little Wavefront .obj mesh loader

#include "stb_image.h"     // Sean Barrett's image loader with Anton's load_texture()

#define _USE_MATH_DEFINES
#define ONE_DEG_IN_RAD (2.0 * M_PI) / 360.0 // 0.017444444

// The view and proj matrices below make-up the camera position, orientation, fov, etc.
// The model matrices moves your surface of revolution to the world coordinate system
extern mat4 view_mat;
extern mat4 proj_mat;
extern mat4 model_mat;

// the vector below indicates camra placement. 
//It looks at (0,0,0) with (0,1,0) as the up-vector
vec3 cam_pos (0.0f, 0.0f, 10.0f);

// Below are the declarations for three functions from stub.cpp 
void loadSurfaceOfRevolution();
void loadUniforms(GLuint shader_programme);
void drawSurfaceOfRevolution();
void keyboardFunction(GLFWwindow* window, int key, int scancode, int action, int mods);

// Forward Declaration
// this function loads a .jpg or .png file into a previously activated texture map
bool load_texture (const char* file_name, GLuint* tex);


/******************************************************************************|
| argc: number of input command-line parameters                                |
| argv: array of input command-line character strings                          |
| EXPECTED INPUT                                                               |
|                                                                              |
| argv[0]: automatically asigned the program name cg05                         |
| argv[1]: Vertex Shader Filename (first argument at command-line)             |
| argv[2]: Fragment Shader Filename (second argument at command-line)          |
| argv[3]: First Texture Filename (third argument at command-line)             |
| argv[4]: Second Texture Filename(optional fourth argument at command-line)   |
|******************************************************************************/

int main (int argc, char *argv[]) {
	if (argc < 4)
	{
		printf("\nEnter atleast 3 filenames to run this program: (1) vtx_shader, (2) frag_shader, (3) texture_1\n");
		printf("\nQuiting now...\n");
		
		return 0;
	}
/*--------------------------------START OPENGL--------------------------------*/
	assert (restart_gl_log ());
	assert (start_gl ());        // Start glfw window with GL context within.

	glfwSetKeyCallback(g_window, keyboardFunction);

/*------------------------------CREATE GEOMETRY-------------------------------*/
	loadSurfaceOfRevolution();

/*-------------------------------CREATE SHADERS-------------------------------*/
    // The vertex shader program generally acts to transform vertices.
    // The fragment shader is where we'll do the actual "shading."
	GLuint shader_programme = create_programme_from_files (
		argv[1], argv[2]
	);

	glUseProgram (shader_programme);
	

/*-------------------------------SETUP TEXTURES-------------------------------*/
	// load textures
	GLuint tex00;
	int tex00location = glGetUniformLocation (shader_programme, "texture00");
	glUniform1i (tex00location, 0);
	glActiveTexture (GL_TEXTURE0);
	assert (load_texture (argv[3], &tex00));

	GLuint tex01;
	int tex01location = glGetUniformLocation (shader_programme, "texture01");
	glUniform1i (tex01location, 1);
	glActiveTexture (GL_TEXTURE1);
	if(argc < 5)
		assert (load_texture (argv[3], &tex01));
	else
		assert (load_texture (argv[4], &tex01));

	
/*-------------------------------CREATE CAMERA--------------------------------*/

        // This sets up the matrix transformations that act as the camera lens
        // and sensor would to transform vertices from view space.
	float near = 0.1f;   // clipping planes
	float far = 1000.0f; 
	float fovy = 35.0f;  // vertical field of view, horiz calculated for aspect
	float aspect = (float)g_gl_width / (float)g_gl_height;      // aspect ratio
	proj_mat = perspective (fovy, aspect, near, far);

	mat4 T = translate (
		identity_mat4 (), vec3 (-cam_pos.v[0], -cam_pos.v[1], -cam_pos.v[2])
	);
	
	// would usually use inverse camera orientation with position to construct
	// view matrix, but for simplicity since camera axis-aligned, not needed here
	view_mat =  T; 
	
/*---------------------------SET RENDERING DEFAULTS---------------------------*/
	// The model matrix stores the position and orientation transformations for your SoR

	model_mat = identity_mat4();


	// Setup basic GL display attributes.	
	glEnable (GL_DEPTH_TEST);   // enable depth-testing
	glDepthFunc (GL_LESS);      // depth-testing interprets a smaller value as "closer"
	glEnable (GL_CULL_FACE);    // cull face
	glCullFace (GL_BACK);       // cull back face
	glFrontFace (GL_CCW);       // set counter-clock-wise vertex order to mean the front
	glClearColor (0.1, 0.1, 0.1, 1.0);   // non-black background to help spot mistakes
	glViewport (0, 0, g_gl_width, g_gl_height); // make sure correct aspect ratio

/*-------------------------------RENDERING LOOP-------------------------------*/
	while (!glfwWindowShouldClose (g_window)) {
		// update timers
		static double previous_seconds = glfwGetTime ();
		double current_seconds = glfwGetTime ();
		double elapsed_seconds = current_seconds - previous_seconds;
		previous_seconds = current_seconds;
		_update_fps_counter (g_window);
		
		// clear graphics context
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		// setup shader use	
		glUseProgram (shader_programme);

		// update and draw mesh, rotating the SoR 
		// set a pace independent of outright rendering speed
                double rotAmt = -60 * elapsed_seconds; 
		model_mat = rotate_y_deg(identity_mat4(), rotAmt) * model_mat;

		// load uniform variables for shaders
		// YOU HAVE TO IMPLEMENT THIS FUNCTION IN stub.cpp	
		loadUniforms(shader_programme);
		
		
		// The following function will actually draw your previously dispatched/loaded Surface of Revolution
		// YOU HAVE TO IMPLEMENT THIS FUNCTION IN stub.cpp	
		drawSurfaceOfRevolution();

		// update other events like input handling 
		glfwPollEvents ();

		aspect = (float)g_gl_width / (float)g_gl_height; // aspect ratio
		proj_mat = perspective (fovy, aspect, near, far);
	  
		// put the stuff we've been drawing onto the display
		glfwSwapBuffers (g_window);
	}
	
	// close GL context and any other GLFW resources
	glfwTerminate();
	return 0;
}

// this function loads a .jpg or .png file into a previously activated texture map
bool load_texture (const char* file_name, GLuint* tex) {
	int x, y, n;
	int force_channels = 4;
	unsigned char* image_data = stbi_load (file_name, &x, &y, &n, force_channels);
	if (!image_data) {
		fprintf (stderr, "ERROR: could not load %s\n", file_name);
		return false;
	}
	// NPOT check
	if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
		fprintf (
			stderr, "WARNING: texture %s is not power-of-2 dimensions\n", file_name
		);
	}
	int width_in_bytes = x * 4;
	unsigned char *top = NULL;
	unsigned char *bottom = NULL;
	unsigned char temp = 0;
	int half_height = y / 2;

	for (int row = 0; row < half_height; row++) {
		top = image_data + row * width_in_bytes;
		bottom = image_data + (y - row - 1) * width_in_bytes;
		for (int col = 0; col < width_in_bytes; col++) {
			temp = *top;
			*top = *bottom;
			*bottom = temp;
			top++;
			bottom++;
		}
	}
	glGenTextures (1, tex);
	glBindTexture (GL_TEXTURE_2D, *tex);
	glTexImage2D (
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		x,
		y,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		image_data
	);
	glGenerateMipmap (GL_TEXTURE_2D);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	GLfloat max_aniso = 0.0f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
	// set the maximum!
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
	return true;
}
