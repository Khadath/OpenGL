#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <vector>
#include <iostream>
#include <random>
#include <sstream>
#include <fstream>
#include <string>

#define TINYPLY_IMPLEMENTATION
#include <tinyply.h>
#include "stl.h"
#include "texture.h"

float distancez = 10.0f;

static void error_callback(int /*error*/, const char* description)
{
	std::cerr << "Error: " << description << std::endl;
}

static void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	std::cout << yoffset << std::endl;
	if (yoffset < 0) {
		distancez += 0.1f;
	}
	if (yoffset > 0) {
		distancez -= 0.1f;
	}
	
}

/* PARTICULES */
struct Particule {
	glm::vec3 position;
	glm::vec3 color;
	glm::vec3 speed;
};

struct Vertex {
	glm::vec3 position;
	glm::vec2 uv;
};

std::vector<Particule> MakeParticules(const int n)
{
	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution01(0, 1);
	std::uniform_real_distribution<float> distributionWorld(-1, 1);

	std::vector<Particule> p;
	p.reserve(n);

	for(int i = 0; i < n; i++)
	{
		p.push_back(Particule{
				{
				distributionWorld(generator),
				distributionWorld(generator),
				distributionWorld(generator)
				},
				{
				distribution01(generator),
				distribution01(generator),
				distribution01(generator)
				},
				{0.f, 0.f, 0.f}
				});
	}

	return p;
}

GLuint MakeShader(GLuint t, std::string path)
{
	std::cout << path << std::endl;
	std::ifstream file(path.c_str(), std::ios::in);
	std::ostringstream contents;
	contents << file.rdbuf();
	file.close();

	const auto content = contents.str();
	std::cout << content << std::endl;

	const auto s = glCreateShader(t);

	GLint sizes[] = {(GLint) content.size()};
	const auto data = content.data();

	glShaderSource(s, 1, &data, sizes);
	glCompileShader(s);

	GLint success;
	glGetShaderiv(s, GL_COMPILE_STATUS, &success);
	if(!success)
	{
		GLchar infoLog[512];
		GLsizei l;
		glGetShaderInfoLog(s, 512, &l, infoLog);

		std::cout << infoLog << std::endl;
	}

	return s;
}

GLuint AttachAndLink(std::vector<GLuint> shaders)
{
	const auto prg = glCreateProgram();
	for(const auto s : shaders)
	{
		glAttachShader(prg, s);
	}

	glLinkProgram(prg);

	GLint success;
	glGetProgramiv(prg, GL_LINK_STATUS, &success);
	if(!success)
	{
		GLchar infoLog[512];
		GLsizei l;
		glGetProgramInfoLog(prg, 512, &l, infoLog);

		std::cout << infoLog << std::endl;
	}

	return prg;
}

void APIENTRY opengl_error_callback(GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar *message,
		const void *userParam)
{
	std::cout << message << std::endl;
}

int main(void)
{
	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

	window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}


	// NOTE: OpenGL error checks have been omitted for brevity
	//apelle de touche 
	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if(!gladLoadGL()) {
		std::cerr << "Something went wrong!" << std::endl;
		exit(-1);
	}

	// Callbacks
	glDebugMessageCallback(opengl_error_callback, nullptr);

	// Shader
	const auto vertex = MakeShader(GL_VERTEX_SHADER, "shader.vert");
	const auto fragment = MakeShader(GL_FRAGMENT_SHADER, "shader.frag");

	const auto program = AttachAndLink({vertex, fragment});

	glUseProgram(program);


	//affichage du logo STL
	vector<Triangle> logo = ReadStl("house_targaryen.stl");

	//Load l'image
	Image texture = LoadImage("wood.bmp");

	vector<Vertex> textures{

		//face 1
		{ glm::vec3(0,0,1),glm::vec2(0,0) },
		{ glm::vec3(0.5,0,1),glm::vec2(1,0) },
		{ glm::vec3(0,0.5,1),glm::vec2(0,1) },

		{ glm::vec3(0.5,0,1),glm::vec2(1,0) },
		{ glm::vec3(0.5,0.5,1),glm::vec2(1,1) },
		{ glm::vec3(0,0.5,1),glm::vec2(0,1) },

		//face 2
		{ glm::vec3(0.5,0,0.5),glm::vec2(0,0) },
		{ glm::vec3(0.5,0,0),glm::vec2(1,0) },
		{ glm::vec3(0.5,0.5,0.5),glm::vec2(0,1) },

		{ glm::vec3(0.5,0,0),glm::vec2(1,0) },
		{ glm::vec3(0.5,0.5,0),glm::vec2(1,1) },
		{ glm::vec3(0.5,0.5,0.5),glm::vec2(0,1) }
	};

	vector<Triangle> SpongeBobHouse = ReadStl("Spongebobs_House.stl");
	vector<Triangle> objs = logo;
	for (int i = 0; i < SpongeBobHouse.size(); i++) {
		objs.push_back(SpongeBobHouse[i]);
	}
	
	// Buffers
	GLuint vbo, vao;
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, textures.size() * sizeof(Vertex) * 6, textures.data(), GL_STATIC_DRAW);

	//position
	const auto index = glGetAttribLocation(program, "position");

	glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
	glEnableVertexAttribArray(index);

	//normal
	const auto indexNormal = glGetAttribLocation(program, "normal");

	glVertexAttribPointer(indexNormal, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 2, (void*)(sizeof(glm::vec3)));
	glEnableVertexAttribArray(indexNormal);

	//Texture
	GLuint textureId = 1;
	glCreateTextures(GL_TEXTURE_2D, 1, &textureId);
	glTextureStorage2D(textureId, 1, GL_RGB8, texture.width, texture.height);
	glTextureSubImage2D(textureId,0,0,0,texture.width,texture.height,GL_RGB,GL_UNSIGNED_BYTE,texture.data.data());

	glBindTextureUnit(0, textureId);
	const auto indextexture = glGetAttribLocation(program, "uv_in");

	glVertexAttribPointer(indextexture, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3)));
	glEnableVertexAttribArray(indextexture);


	int uniformTexture = glGetUniformLocation(program, "Texture");
	glProgramUniform1i(program, uniformTexture, 0);

	//FRAME BUFFER
	GLuint frameBufferId = 1;
	glCreateFramebuffers(1, &frameBufferId);
	//glNamedFramebufferTexture()
	//glBindframebuffer


	float scale = 0.01f;
	glm::mat4 scaleMat = glm::scale(glm::vec3(scale, scale, scale));

	float rotate = 0.0f;
	float angle = 90.0f;
	float rad = angle * 180.0 / 3.14;

	//glEnable(GL_DEPTH_TEST);

	while (!glfwWindowShouldClose(window))
	{


		glfwSetScrollCallback(window, scroll_callback);

		int width, height;

		glfwGetFramebufferSize(window, &width, &height);

		glViewport(0, 0, width, height);

		glClear(GL_COLOR_BUFFER_BIT);
		//HOUSE TARGARYAN
		
			//couleur
			glm::vec3 green = glm::vec3(0.f, 1.f, 1.f);
			int uniformCouleur = glGetUniformLocation(program, "color");
			glProgramUniform3f(program, uniformCouleur, green.x, green.y, green.z);
		
			//-----------------------------TRANSFORM-----------------------------
			rotate += 0.05f;

			glm::mat4 rotateMat = glm::rotate(angle, glm::vec3(0.0f, 1.0f, 1.0f));
			glm::mat4 mat = scaleMat * rotateMat;
			int uniformTransform = glGetUniformLocation(program, "Mat");
			glProgramUniformMatrix4fv(program, uniformTransform, 1, GL_FALSE, &mat[0][0]);

			//-----------------------------PROJECTION-----------------------------
			glm::mat4 projection = glm::perspective(rad, float(width / height), 0.1f, 1000.0f);
			int uniformProjection = glGetUniformLocation(program, "projection");
			glProgramUniformMatrix4fv(program, uniformProjection, 1, GL_FALSE, &projection[0][0]);

			//-----------------------------VIEW-----------------------------
			glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, distancez), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			int uniformView = glGetUniformLocation(program, "view");
			glProgramUniformMatrix4fv(program, uniformView, 1, GL_FALSE, &view[0][0]);

			//glClear(GL_DEPTH_BUFFER_BIT);

			glDrawArrays(GL_TRIANGLES, 0, logo.size() * 3);


		//DEUXIEME OBJET
		
			//couleur
			glm::vec3 blue = glm::vec3(0.f, 0.f, 1.f);
			//uniformCouleur = glGetUniformLocation(program, "color");
			glProgramUniform3f(program, uniformCouleur, blue.x, blue.y, blue.z);

			//-----------------------------TRANSFORM-----------------------------
			scaleMat = glm::scale(glm::vec3(0.02f, 0.02f, 0.02f));
			mat = scaleMat;
			//uniformTransform = glGetUniformLocation(program, "Mat");
			glProgramUniformMatrix4fv(program, uniformTransform, 1, GL_FALSE, &mat[0][0]);

			//-----------------------------PROJECTION-----------------------------
			//uniformProjection = glGetUniformLocation(program, "projection");
			glProgramUniformMatrix4fv(program, uniformProjection, 1, GL_FALSE, &projection[0][0]);

			//-----------------------------VIEW-----------------------------
			//uniformView = glGetUniformLocation(program, "view");
			glProgramUniformMatrix4fv(program, uniformView, 1, GL_FALSE, &view[0][0]);

			glDrawArrays(GL_TRIANGLES, logo.size() * 3, objs.size() * 3 - (logo.size() * 3));
			
			
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
