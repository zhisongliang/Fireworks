/*
CPE/CSC 471 Lab base code Wood/Dunn/Eckhardt
modified by: Zhisong Liang
*/

#include <iostream>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include <time.h>

#include "WindowManager.h"
#include "Shape.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include "Rocket.cpp"
#include "Sparkle.cpp"
#include "Trace.cpp"

using namespace std;
using namespace glm;
shared_ptr<Shape> shape;

#define PI 3.14159265358979323846
#define NUM_ROCKETS 30
#define NUM_SPARKLES 300
#define NUM_TRACES 500

Rocket rockets[NUM_ROCKETS];
Sparkle sparkles[NUM_ROCKETS * NUM_SPARKLES];
Trace traces[NUM_ROCKETS * NUM_TRACES];

double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime = glfwGetTime();
	double difference = actualtime - lasttime;
	lasttime = actualtime;
	return difference;
}
class camera
{
public:
	glm::vec3 pos, rot;
	int w, a, s, d;
	camera()
	{
		w = a = s = d = 0;
		pos = rot = glm::vec3(0, 0, 0);
	}
	glm::mat4 process(double ftime)
	{
		float speed = 0;
		if (w == 1)
		{
			speed = 10 * ftime;
		}
		else if (s == 1)
		{
			speed = -10 * ftime;
		}
		float yangle = 0;
		if (a == 1)
			yangle = -3 * ftime;
		else if (d == 1)
			yangle = 3 * ftime;
		rot.y += yangle;
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::vec4 dir = glm::vec4(0, 0, speed, 1);
		dir = dir * R;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return R * T;
	}
};

camera mycam;

class Application : public EventCallbacks
{
	// compart distance of two vectors pos from camera, draw the furthest first
	static bool compareFunc(vec4 i, vec4 j) { return (i.w > j.w); }

public:
	WindowManager *windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog, psky, postproc, particles, trace;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID, VertexArrayIDScreen, VertexArrayIDParticles, VertexArrayIDTrace;

	// Data necessary to give our box to OpenGL
	GLuint VertexBufferID, VertexBufferTexScreen, VertexBufferIDScreen, VertexNormDBox, VertexTexBox, IndexBufferIDBox, InstanceBuffer;

	// data for particles
	GLuint VertexBufferIDParticles, VertexNormDParticles, VertexTexBoxParticles, IndexBufferIDBoxParticles, InstanceBufferParticles;
	GLuint InstanceBufferParticlesColor;

	GLuint VertexBufferIDTrace, VertexNormDTrace, VertexTexBoxTrace, IndexBufferIDBoxTrace, InstanceBufferTrace;
	GLuint InstanceBufferTraceColor;

	// framebufferstuff
	GLuint fb, depth_fb, FBOtex;
	// texture data
	GLuint Texture;
	GLuint Texture2;

	vec4 rocketPositions[NUM_ROCKETS];

	vec4 sparklePositions[NUM_SPARKLES * NUM_ROCKETS];
	vec4 sparkleColor[NUM_SPARKLES * NUM_ROCKETS];

	vec4 tracePositions[NUM_TRACES * NUM_ROCKETS];
	vec4 traceColor[NUM_TRACES * NUM_ROCKETS];

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		// if (key == GLFW_KEY_A && action == GLFW_PRESS)
		// {
		// 	mycam.a = 1;
		// }
		// if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		// {
		// 	mycam.a = 0;
		// }
		// if (key == GLFW_KEY_D && action == GLFW_PRESS)
		// {
		// 	mycam.d = 1;
		// }
		// if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		// {
		// 	mycam.d = 0;
		// }
	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;
		float newPt[2];
		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			std::cout << "Pos X " << posX << " Pos Y " << posY << std::endl;

			// // change this to be the points converted to WORLD
			// // THIS IS BROKEN< YOU GET TO FIX IT - yay!
			// newPt[0] = 0;
			// newPt[1] = 0;

			// std::cout << "converted:" << newPt[0] << " " << newPt[1] << std::endl;
			// glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
			// // update the vertex array with the updated points
			// glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * 6, sizeof(float) * 2, newPt);
			// glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
	}

	// updates the instance buffer of all rockets
	void updateAllRocketInstanceBuffer()
	{
		glBindBuffer(GL_ARRAY_BUFFER, InstanceBuffer);
		// (target, offset, size, data)
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec4) * NUM_ROCKETS, rocketPositions);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// updates the instance buffer of all rockets
	void updateAllSparklesInstanceBuffer()
	{
		glBindBuffer(GL_ARRAY_BUFFER, InstanceBufferParticles);
		// (target, offset, size, data)
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec4) * NUM_ROCKETS * NUM_SPARKLES, sparklePositions);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void updateAllTraceInstanceBuffer()
	{
		glBindBuffer(GL_ARRAY_BUFFER, InstanceBufferTrace);
		// (target, offset, size, data)
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec4) * NUM_ROCKETS * NUM_TRACES, tracePositions);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// // updates the instance buffer of a single rocket
	// void updateInstanceBuffer(int index)
	// {
	// 	glBindBuffer(GL_ARRAY_BUFFER, InstanceBuffer);
	// 	// (target, offset, size, data)
	// 	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * index, sizeof(vec4), &rocketPostions[index]);
	// 	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// }

	// if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		// get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{

		string resourceDirectory = "../../resources";
		// Initialize mesh.
		shape = make_shared<Shape>();
		shape->loadMesh(resourceDirectory + "/sphere.obj");
		shape->resize();
		shape->init();

		// screen plane
		glGenVertexArrays(1, &VertexArrayIDScreen);
		glBindVertexArray(VertexArrayIDScreen);
		// generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferIDScreen);
		// set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferIDScreen);
		vec3 vertices[6];
		vertices[0] = vec3(-1, -1, 0);
		vertices[1] = vec3(1, -1, 0);
		vertices[2] = vec3(1, 1, 0);
		vertices[3] = vec3(-1, -1, 0);
		vertices[4] = vec3(1, 1, 0);
		vertices[5] = vec3(-1, 1, 0);
		// actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(vec3), vertices, GL_STATIC_DRAW);
		// we need to set up the vertex array
		glEnableVertexAttribArray(0);
		// key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
		// generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferTexScreen);
		// set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferTexScreen);
		vec2 texscreen[6];
		texscreen[0] = vec2(0, 0);
		texscreen[1] = vec2(1, 0);
		texscreen[2] = vec2(1, 1);
		texscreen[3] = vec2(0, 0);
		texscreen[4] = vec2(1, 1);
		texscreen[5] = vec2(0, 1);
		// actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(vec2), texscreen, GL_STATIC_DRAW);
		// we need to set up the vertex array
		glEnableVertexAttribArray(1);
		// key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
		glBindVertexArray(0);

		// generate the VAO
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		// generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferID);
		// set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);

		GLfloat rect_vertices[] = {
			// front
			-1.0, -1.0, 1.0, // LD
			1.0, -1.0, 1.0,	 // RD
			1.0, 1.0, 1.0,	 // RU
			-1.0, 1.0, 1.0,	 // LU
		};
		// make it a bit smaller
		for (int i = 0; i < 12; i++)
			rect_vertices[i] *= 0.5;
		// actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(rect_vertices), rect_vertices, GL_DYNAMIC_DRAW);

		// we need to set up the vertex array
		glEnableVertexAttribArray(0);
		// key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		// color
		GLfloat rect_norm[] = {
			// front colors
			0.0,
			0.0,
			1.0,
			0.0,
			0.0,
			1.0,
			0.0,
			0.0,
			1.0,
			0.0,
			0.0,
			1.0,
		};
		glGenBuffers(1, &VertexNormDBox);
		// set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexNormDBox);
		glBufferData(GL_ARRAY_BUFFER, sizeof(rect_norm), rect_norm, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		// color
		glm::vec2 rect_tex[] = {
			// front colors
			glm::vec2(0.0, 1.0),
			glm::vec2(1.0, 1.0),
			glm::vec2(1.0, 0.0),
			glm::vec2(0.0, 0.0),
		};
		glGenBuffers(1, &VertexTexBox);
		// set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexTexBox);
		glBufferData(GL_ARRAY_BUFFER, sizeof(rect_tex), rect_tex, GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

		glGenBuffers(1, &IndexBufferIDBox);
		// set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		GLushort rect_elements[] = {
			// front
			0,
			1,
			2,
			2,
			3,
			0,
		};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rect_elements), rect_elements, GL_STATIC_DRAW);

		// generate vertex buffer to hand off to OGL ###########################
		glGenBuffers(1, &InstanceBuffer);
		// set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, InstanceBuffer);

		for (int i = 0; i < NUM_ROCKETS; i++)
		{
			// new a rocket
			rockets[i] = Rocket();
			rocketPositions[i] = rockets[i].getPos();
		}

		// actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, NUM_ROCKETS * sizeof(glm::vec4), rocketPositions, GL_STATIC_DRAW);

		// which is 3 in this case, because in shader layout (location = 3) in vec4 InstancePos;
		int position_loc = glGetAttribLocation(prog->pid, "InstancePos");
		// for (int i = 0; i < NUM_ROCKETS; i++)
		// {
		// Set up the vertex attribute
		glVertexAttribPointer(position_loc,			 // Location
							  4, GL_FLOAT, GL_FALSE, // vec4
							  sizeof(vec4),			 // Stride
							  0);					 // Start offset
													 // Enable it
		glEnableVertexAttribArray(position_loc);
		// Make it instanced, this is per instance, not per vertex
		glVertexAttribDivisor(position_loc, 1);
		//}

		glBindVertexArray(0);

		/* FOR PARTICLES START*/
		// VAO
		glGenVertexArrays(1, &VertexArrayIDParticles);
		glBindVertexArray(VertexArrayIDParticles);

		// VBO
		// generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferIDParticles);
		// set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferIDParticles);

		// actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(rect_vertices), rect_vertices, GL_DYNAMIC_DRAW);

		// we need to set up the vertex array
		glEnableVertexAttribArray(0);
		// key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		glGenBuffers(1, &VertexNormDParticles);
		// set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexNormDParticles);
		glBufferData(GL_ARRAY_BUFFER, sizeof(rect_norm), rect_norm, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		glGenBuffers(1, &VertexTexBoxParticles);
		// set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexTexBoxParticles);
		glBufferData(GL_ARRAY_BUFFER, sizeof(rect_tex), rect_tex, GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

		glGenBuffers(1, &IndexBufferIDBoxParticles);
		// set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBoxParticles);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rect_elements), rect_elements, GL_STATIC_DRAW);

		// generate vertex buffer to hand off to OGL ###########################
		glGenBuffers(1, &InstanceBufferParticles);
		// set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, InstanceBufferParticles);

		for (int i = 0; i < NUM_ROCKETS; i++)
		{
			float x = rockets[i].getPos().x;
			float y = rockets[i].getPos().y;
			float z = rockets[i].getPos().z;
			for (int j = 0; j < NUM_SPARKLES; j++)
			{
				// new a Sparkle
				sparkles[i * NUM_SPARKLES + j] = Sparkle(x, y, z, rockets[i].getExplosionRadius());
				sparklePositions[i * NUM_SPARKLES + j] = sparkles[i * NUM_SPARKLES + j].getPos();
				// pass random color to each sparkle
				sparkleColor[i * NUM_SPARKLES + j] = glm::vec4(rand() % 100 / 100.0f, rand() % 100 / 100.0f, rand() % 100 / 100.0f, 1.0f);
			}
		}

		// actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, NUM_ROCKETS * NUM_SPARKLES * sizeof(glm::vec4), sparklePositions, GL_STATIC_DRAW);

		// which is 3 in this case, because in shader layout (location = 3) in vec4 InstancePos;
		position_loc = glGetAttribLocation(particles->pid, "InstancePos");
		// for (int i = 0; i < NUM_ROCKETS; i++)
		// {
		// Set up the vertex attribute
		glVertexAttribPointer(position_loc,			 // Location
							  4, GL_FLOAT, GL_FALSE, // vec4
							  sizeof(vec4),			 // Stride
							  0);					 // Start offset
													 // Enable it
		glEnableVertexAttribArray(position_loc);
		// Make it instanced, this is per instance, not per vertex
		glVertexAttribDivisor(position_loc, 1);
		//}

		// generate vertex buffer to hand off to OGL ###########################
		glGenBuffers(1, &InstanceBufferParticlesColor);
		// set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, InstanceBufferParticlesColor);

		glBufferData(GL_ARRAY_BUFFER, NUM_ROCKETS * NUM_SPARKLES * sizeof(glm::vec4), sparkleColor, GL_STATIC_DRAW);

		position_loc = glGetAttribLocation(particles->pid, "InstanceColor");

		// Set up the vertex attribute
		glVertexAttribPointer(position_loc,			 // Location
							  4, GL_FLOAT, GL_FALSE, // vec4
							  sizeof(vec4),			 // Stride
							  0);					 // Start offset
													 // Enable it
		glEnableVertexAttribArray(position_loc);
		// Make it instanced, this is per instance, not per vertex
		glVertexAttribDivisor(position_loc, 1);

		glBindVertexArray(0);
		/*FOR PARTICLES END*/

		/* FOR Trace START*/
		// VAO
		glGenVertexArrays(1, &VertexArrayIDTrace);
		glBindVertexArray(VertexArrayIDTrace);

		// VBO
		// generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferIDTrace);
		// set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferIDTrace);

		// actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(rect_vertices), rect_vertices, GL_DYNAMIC_DRAW);

		// we need to set up the vertex array
		glEnableVertexAttribArray(0);
		// key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		glGenBuffers(1, &VertexNormDTrace);
		// set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexNormDTrace);
		glBufferData(GL_ARRAY_BUFFER, sizeof(rect_norm), rect_norm, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		glGenBuffers(1, &VertexTexBoxTrace);
		// set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexTexBoxTrace);
		glBufferData(GL_ARRAY_BUFFER, sizeof(rect_tex), rect_tex, GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

		glGenBuffers(1, &IndexBufferIDBoxTrace);
		// set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBoxTrace);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rect_elements), rect_elements, GL_STATIC_DRAW);

		// generate vertex buffer to hand off to OGL ###########################
		glGenBuffers(1, &InstanceBufferTrace);
		// set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, InstanceBufferTrace);

		for (int i = 0; i < NUM_ROCKETS; i++)
		{
			float x = rockets[i].getPos().x;
			float y = rockets[i].getPos().y;
			float z = rockets[i].getPos().z;
			for (int j = 0; j < NUM_TRACES; j++)
			{
				// new a Sparkle
				traces[i * NUM_TRACES + j] = Trace(x, y, z);
				tracePositions[i * NUM_TRACES + j] = traces[i * NUM_TRACES + j].getPos();
				// pass random color to each sparkle
				traceColor[i * NUM_TRACES + j] = glm::vec4(rand() % 100 / 100.0f, rand() % 100 / 100.0f, rand() % 100 / 100.0f, 1.0f);
			}
		}

		// actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, NUM_TRACES * NUM_ROCKETS * sizeof(glm::vec4), tracePositions, GL_STATIC_DRAW);

		// which is 3 in this case, because in shader layout (location = 3) in vec4 InstancePos;
		position_loc = glGetAttribLocation(particles->pid, "InstancePos");
		// for (int i = 0; i < NUM_ROCKETS; i++)
		// {
		// Set up the vertex attribute
		glVertexAttribPointer(position_loc,			 // Location
							  4, GL_FLOAT, GL_FALSE, // vec4
							  sizeof(vec4),			 // Stride
							  0);					 // Start offset
													 // Enable it
		glEnableVertexAttribArray(position_loc);
		// Make it instanced, this is per instance, not per vertex
		glVertexAttribDivisor(position_loc, 1);
		//}

		// generate vertex buffer to hand off to OGL ###########################
		glGenBuffers(1, &InstanceBufferTraceColor);
		// set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, InstanceBufferTraceColor);

		glBufferData(GL_ARRAY_BUFFER, NUM_TRACES * NUM_ROCKETS * sizeof(glm::vec4), traceColor, GL_STATIC_DRAW);

		position_loc = glGetAttribLocation(particles->pid, "InstanceColor");

		// Set up the vertex attribute
		glVertexAttribPointer(position_loc,			 // Location
							  4, GL_FLOAT, GL_FALSE, // vec4
							  sizeof(vec4),			 // Stride
							  0);					 // Start offset
													 // Enable it
		glEnableVertexAttribArray(position_loc);
		// Make it instanced, this is per instance, not per vertex
		glVertexAttribDivisor(position_loc, 1);

		glBindVertexArray(0);
		/*FOR Trace END*/

		int width, height, channels;
		char filepath[1000];

		// texture 1
		string str = resourceDirectory + "/Blue_Giant.jpg";
		strcpy(filepath, str.c_str());
		unsigned char *data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		// texture 2
		str = resourceDirectory + "/night.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//[TWOTEXTURES]
		// set the 2 textures to the correct samplers in the fragment shader:
		GLuint Tex1Location = glGetUniformLocation(prog->pid, "tex"); // tex, tex2... sampler in the fragment shader
		GLuint Tex2Location = glGetUniformLocation(prog->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(prog->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);

		Tex1Location = glGetUniformLocation(postproc->pid, "tex"); // tex, tex2... sampler in the fragment shader
		glUseProgram(postproc->pid);
		glUniform1i(Tex1Location, 0);

		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		// RGBA8 2D texture, 24 bit depth texture, 256x256
		glGenTextures(1, &FBOtex);
		glBindTexture(GL_TEXTURE_2D, FBOtex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// NULL means reserve texture memory, but texels are undefined
		//**** Tell OpenGL to reserve level 0
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
		// You must reserve memory for other mipmaps levels as well either by making a series of calls to
		// glTexImage2D or use glGenerateMipmapEXT(GL_TEXTURE_2D).
		// Here, we'll use :
		glGenerateMipmap(GL_TEXTURE_2D);
		// make a frame buffer
		//-------------------------
		glGenFramebuffers(1, &fb);
		glBindFramebuffer(GL_FRAMEBUFFER, fb);
		// Attach 2D texture to this FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBOtex, 0);
		//-------------------------
		glGenRenderbuffers(1, &depth_fb);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_fb);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
		//-------------------------
		// Attach depth buffer to FBO
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_fb);
		//-------------------------
		// Does the GPU support current FBO configuration?
		GLenum status;
		status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		switch (status)
		{
		case GL_FRAMEBUFFER_COMPLETE:
			cout << "status framebuffer: good";
			break;
		default:
			cout << "status framebuffer: bad!!!!!!!!!!!!!!!!!!!!!!!!!";
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// General OGL initialization - set OGL state here
	void init(const std::string &resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		// glDisable(GL_DEPTH_TEST);
		//  Initialize the GLSL program.
		prog = std::make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
		if (!prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("campos");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");
		prog->addAttribute("InstancePos");

		psky = std::make_shared<Program>();
		psky->setVerbose(true);
		psky->setShaderNames(resourceDirectory + "/skyvertex.glsl", resourceDirectory + "/skyfrag.glsl");
		if (!psky->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		psky->addUniform("P");
		psky->addUniform("V");
		psky->addUniform("M");
		psky->addUniform("campos");
		psky->addAttribute("vertPos");
		psky->addAttribute("vertNor");
		psky->addAttribute("vertTex");

		// program for the postprocessing
		postproc = std::make_shared<Program>();
		postproc->setVerbose(true);
		postproc->setShaderNames(resourceDirectory + "/postproc_vertex.glsl", resourceDirectory + "/postproc_fragment.glsl");
		if (!postproc->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		postproc->addAttribute("vertPos");
		postproc->addAttribute("vertTex");

		// program for the particles
		particles = std::make_shared<Program>();
		particles->setVerbose(true);
		particles->setShaderNames(resourceDirectory + "/particle_vertex.glsl", resourceDirectory + "/particle_fragment.glsl");
		if (!particles->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		particles->addUniform("P");
		particles->addUniform("V");
		particles->addUniform("M");
		particles->addUniform("campos");
		particles->addAttribute("vertPos");
		particles->addAttribute("vertNor");
		particles->addAttribute("vertTex");
		particles->addAttribute("InstancePos");
		particles->addAttribute("InstanceColor");
	}
	static float random(float min, float max)
	{
		return ((float)rand() / RAND_MAX) * (max - min) + min;
	}

	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render_to_framebuffer()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fb);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		double frametime = get_last_elapsed_time();

		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now

		glm::mat4 V, M, P; // View, Model and Perspective matrix
		V = mycam.process(frametime);
		M = glm::mat4(1);
		// Apply orthographic projection....
		P = glm::ortho(-1 * aspect, 1 * aspect, -1.0f, 1.0f, -2.0f, 100.0f);
		if (width < height)
		{
			P = glm::ortho(-1.0f, 1.0f, -1.0f / aspect, 1.0f / aspect, -2.0f, 100.0f);
		}
		// ...but we overwrite it (optional) with a perspective projection.
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width / (float)height), 0.1f, 1000.0f); // so much type casting... GLM metods are quite funny ones
		float sangle = 3.1415926 / 2.;
		glm::mat4 RotateXSky = glm::rotate(glm::mat4(1.0f), sangle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::vec3 camp = -mycam.pos;
		glm::mat4 TransSky = glm::translate(glm::mat4(1.0f), camp);
		glm::mat4 SSky = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f));

		M = TransSky * RotateXSky * SSky;

		// Draw the box using GLSL.
		psky->bind();

		// send the matrices to the shaders
		glUniformMatrix4fv(psky->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(psky->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(psky->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(psky->getUniform("campos"), 1, &mycam.pos[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glDisable(GL_DEPTH_TEST);
		shape->draw(psky, false);
		glEnable(GL_DEPTH_TEST);

		psky->unbind();

		// animation with the model matrix:
		static float w = 0.0;
		w += 1.0 * frametime; // rotation angle
		float trans = 0;	  // sin(t) * 2;
		glm::mat4 RotateY = glm::rotate(glm::mat4(1.0f), w, glm::vec3(0.0f, 1.0f, 0.0f));
		float angle = -3.1415926 / 2.0;
		glm::mat4 RotateX = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3 + trans));
		glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f));

		M = TransZ * RotateY * RotateX * S;

		// Draw the box using GLSL.
		prog->bind();

		// send the matrices to the shaders
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);

		glBindVertexArray(VertexArrayID);
		// actually draw from vertex 0, 3 vertices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		// glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);
		mat4 Vi = glm::transpose(V);
		Vi[0][3] = 0;
		Vi[1][3] = 0;
		Vi[2][3] = 0;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -100.0f));

		S = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
		M = TransZ * S;
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);

		// update rocket status
		for (int i = 0; i < NUM_ROCKETS; i++)
		{
			rockets[i].updateDelayLaunch();
			if (rockets[i].ready() == 1)
			{
				rocketPositions[i].y = rockets[i].getPos().y;
				rockets[i].updatePos();
				// if rocket explosed, reinit it
				if (rockets[i].isExplosed() == 1)
				{
					rockets[i].updateLife();
				}
				if (rockets[i].getTimeAfterExplosion() < 0)
				{
					rockets[i].reinitPos();
					rocketPositions[i] = rockets[i].getPos();
					for (int j = 0; j < NUM_SPARKLES; j++)
					{
						sparkles[i * NUM_SPARKLES + j].reinit(rocketPositions[i].x, rocketPositions[i].y, rocketPositions[i].z, rockets[i].getExplosionRadius());
						sparklePositions[i * NUM_SPARKLES + j] = rocketPositions[i];
						// sparkleColor[i * NUM_SPARKLES + j].a = 1.0;
					}
				}
			}
		}

		updateAllRocketInstanceBuffer();

		glDisable(GL_DEPTH_TEST);
		/// rocket
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void *)0, NUM_ROCKETS);
		glEnable(GL_DEPTH_TEST);

		glBindVertexArray(0);

		prog->unbind();

		// sparkles
		particles->bind();

		// send the matrices to the shaders
		glUniformMatrix4fv(particles->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(particles->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(particles->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(particles->getUniform("campos"), 1, &mycam.pos[0]);

		glBindVertexArray(VertexArrayIDParticles);
		// actually draw from vertex 0, 3 vertices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBoxParticles);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -100.0f));

		S = glm::scale(glm::mat4(1.0f), glm::vec3(.4f, .4f, .4f));
		M = TransZ * S;
		glUniformMatrix4fv(particles->getUniform("M"), 1, GL_FALSE, &M[0][0]);

		glDisable(GL_DEPTH_TEST);

		// update sparkles status
		for (int i = 0; i < NUM_ROCKETS; i++)
		{
			for (int j = 0; j < NUM_SPARKLES; j++)
			{
				sparklePositions[i * NUM_SPARKLES + j].x = sparkles[i * NUM_SPARKLES + j].getPos().x;
				sparklePositions[i * NUM_SPARKLES + j].y = sparkles[i * NUM_SPARKLES + j].getPos().y;
				sparkles[i * NUM_SPARKLES + j].updatePos(rockets[i].isExplosed(), rockets[i].getPos().y);

				// if sparkle died, reinit it
				if (sparkles[i * NUM_SPARKLES + j].getDied() == 1)
				{
					// move it away from the camera
					sparklePositions[i * NUM_SPARKLES + j].x = -100;
				}
			}
		}

		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void *)0, NUM_SPARKLES * NUM_ROCKETS);

		updateAllSparklesInstanceBuffer();

		glBindVertexArray(0);

		/*** TRACE ***/
		glBindVertexArray(VertexArrayIDTrace);
		// actually draw from vertex 0, 3 vertices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBoxTrace);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -100.0f));

		S = glm::scale(glm::mat4(1.0f), glm::vec3(.02f, .02f, .02f));
		M = TransZ * S;
		glUniformMatrix4fv(particles->getUniform("M"), 1, GL_FALSE, &M[0][0]);

		glEnable(GL_DEPTH_TEST);

		// update trace status
		for (int i = 0; i < NUM_ROCKETS; i++)
		{
			for (int j = 0; j < NUM_TRACES; j++)
			{
				tracePositions[i * NUM_TRACES + j].x = traces[i * NUM_TRACES + j].getPos().x;
				tracePositions[i * NUM_TRACES + j].y = traces[i * NUM_TRACES + j].getPos().y;
				float randomX;
				float randomY;
				if (j % 2 == 0)
				{
					randomX = random(-0.2, 0.2);
					randomY = random(0.01, 1.0);
				}
				else if (j % 3 == 0)
				{
					randomX = random(-0.12, 0.12);
					randomY = random(1.0, 4.0);
				}
				else if (j % 5 == 0)
				{
					randomX = random(-0.7, 0.7);
					randomY = random(4.0, 8.0);
				}

				traces[i * NUM_TRACES + j].updatePos(rockets[i].getPos().x - randomX,
													 rockets[i].getPos().y - randomY,
													 rockets[i].getPos().z);

				// if rocket explosed
				if (rockets[i].isExplosed() == 1)
				{
					// move it away from the camera
					tracePositions[i * NUM_TRACES + j].x = -100;
				}
			}
		}

		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void *)0, NUM_TRACES * NUM_ROCKETS);

		updateAllTraceInstanceBuffer();

		glBindVertexArray(0);
		particles->unbind();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, FBOtex);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	void render()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);
		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		postproc->bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, FBOtex);
		glBindVertexArray(VertexArrayIDScreen);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		postproc->unbind();
	}
};
//******************************************************************************************

//******************************************************************************************
int main(int argc, char **argv)
{
	srand(time(NULL));
	std::string resourceDir = "../../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager *windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom();
	// application->initParticles();

	// Loop until the user closes the window.
	while (!glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render_to_framebuffer();
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
