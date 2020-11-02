/*
Student Information
*/
#define _USE_MATH_DEFINES
#include <cmath>


#include "Dependencies/glew/glew.h"
#include "Dependencies/GLFW/glfw3.h"
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#include "Dependencies/glm/gtc/type_ptr.hpp"

#include "Shader.h"
#include "Texture.h"



#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>

// screen setting
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;


//mouse setting
bool IsMouseLeftPress = false;
bool IsMouseRightPress = false;
bool IsFirstMouse = true;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float yaw = 90.0f;	
float pitch = 0.0f;


//light
float lightX = 0.0f, lightY = 200.0f, lightZ = 1000.0f;
float ambientLightCounter = 1.0f;
float diffCol = 1.2f, specCol = 0.5f;

//object

	// dolphin
	GLuint dolphinVAO, dolphinIndicesSize;
	float dolphinZ = 0.0f , dolphinRotateY = 90.0f;
	Texture dolphinTexture1, dolphinTexture2;
	int dolphinTextureSelect = 1;

	//sea
	GLuint seaVAO, seaIndicesSize;
	Texture seaTexture1, seaTexture2;
	int seaTextureSelect = 1;

	//block
	GLuint blockVAO, blockIndicesSize;
	Texture blockTexture;
	float boxX = 0.0f, boxZ = 0.0f;



Shader shader;
Shader shader2;

//camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 500.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraTop = glm::vec3(0.0f, 1.0f, 0.0f);




// struct for storing the obj file
struct Vertex {
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
};

struct Model {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

Model loadOBJ(const char* objPath)
{
	// function to load the obj file
	// Note: this simple function cannot load all obj files.

	struct V {
		// struct for identify if a vertex has showed up
		unsigned int index_position, index_uv, index_normal;
		bool operator == (const V& v) const {
			return index_position == v.index_position && index_uv == v.index_uv && index_normal == v.index_normal;
		}
		bool operator < (const V& v) const {
			return (index_position < v.index_position) ||
				(index_position == v.index_position && index_uv < v.index_uv) ||
				(index_position == v.index_position && index_uv == v.index_uv && index_normal < v.index_normal);
		}
	};

	std::vector<glm::vec3> temp_positions;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	std::map<V, unsigned int> temp_vertices;

	Model model;
	unsigned int num_vertices = 0;

	std::cout << "\nLoading OBJ file " << objPath << "..." << std::endl;

	std::ifstream file;
	file.open(objPath);

	// Check for Error
	if (file.fail()) {
		std::cerr << "Impossible to open the file! Do you use the right path? See Tutorial 6 for details" << std::endl;
		exit(1);
	}

	while (!file.eof()) {
		// process the object file
		char lineHeader[128];
		file >> lineHeader;

		if (strcmp(lineHeader, "v") == 0) {
			// geometric vertices
			glm::vec3 position;
			file >> position.x >> position.y >> position.z;
			temp_positions.push_back(position);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			// texture coordinates
			glm::vec2 uv;
			file >> uv.x >> uv.y;
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			// vertex normals
			glm::vec3 normal;
			file >> normal.x >> normal.y >> normal.z;
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			// Face elements
			V vertices[3];
			for (int i = 0; i < 3; i++) {
				char ch;
				file >> vertices[i].index_position >> ch >> vertices[i].index_uv >> ch >> vertices[i].index_normal;
			}

			// Check if there are more than three vertices in one face.
			std::string redundency;
			std::getline(file, redundency);
			if (redundency.length() >= 5) {
				std::cerr << "There may exist some errors while load the obj file. Error content: [" << redundency << " ]" << std::endl;
				std::cerr << "Please note that we only support the faces drawing with triangles. There are more than three vertices in one face." << std::endl;
				std::cerr << "Your obj file can't be read properly by our simple parser :-( Try exporting with other options." << std::endl;
				exit(1);
			}

			for (int i = 0; i < 3; i++) {
				if (temp_vertices.find(vertices[i]) == temp_vertices.end()) {
					// the vertex never shows before
					Vertex vertex;
					vertex.position = temp_positions[vertices[i].index_position - 1];
					vertex.uv = temp_uvs[vertices[i].index_uv - 1];
					vertex.normal = temp_normals[vertices[i].index_normal - 1];

					model.vertices.push_back(vertex);
					model.indices.push_back(num_vertices);
					temp_vertices[vertices[i]] = num_vertices;
					num_vertices += 1;
				}
				else {
					// reuse the existing vertex
					unsigned int index = temp_vertices[vertices[i]];
					model.indices.push_back(index);
				}
			} // for
		} // else if
		else {
			// it's not a vertex, texture coordinate, normal or face
			char stupidBuffer[1024];
			file.getline(stupidBuffer, 1024);
		}
	}
	file.close();

	std::cout << "There are " << num_vertices << " vertices in the obj file.\n" << std::endl;
	return model;
}

void get_OpenGL_info()
{
	// OpenGL information
	const GLubyte* name = glGetString(GL_VENDOR);
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* glversion = glGetString(GL_VERSION);
	std::cout << "OpenGL company: " << name << std::endl;
	std::cout << "Renderer name: " << renderer << std::endl;
	std::cout << "OpenGL version: " << glversion << std::endl;
}


void SendDolphinData()
{
	Model dolphin = loadOBJ("resources/dolphin/dolphin.obj");

	glGenVertexArrays(1, &dolphinVAO);
	glBindVertexArray(dolphinVAO);

	GLuint dolphinVBO;
	glGenBuffers(1, &dolphinVBO);
	glBindBuffer(GL_ARRAY_BUFFER, dolphinVBO);
	glBufferData(GL_ARRAY_BUFFER, dolphin.vertices.size() * sizeof(Vertex), &dolphin.vertices[0], GL_STATIC_DRAW);



	glEnableVertexAttribArray(0);

	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, position) // array buffer offset
	);

	glEnableVertexAttribArray(1);

	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, uv) // array buffer offset
	);

	glEnableVertexAttribArray(2);

	glVertexAttribPointer(
		2, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, normal) // array buffer offset
	);

	GLuint dolphinEBO;
	glGenBuffers(1, &dolphinEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dolphinEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, dolphin.indices.size() * sizeof(unsigned int), &dolphin.indices[0], GL_STATIC_DRAW);


	std::cout << "dolphin.indices.size: " << dolphin.indices.size() << std::endl;
	dolphinIndicesSize = dolphin.indices.size();

	dolphinTexture1.setupTexture("resources/dolphin/dolphin_01.jpg");
	dolphinTexture2.setupTexture("resources/dolphin/dolphin_02.jpg");

}


void SendSeaData() {
	Model sea = loadOBJ("resources/sea/sea.obj");
	glGenVertexArrays(1, &seaVAO);
	glBindVertexArray(seaVAO);

	GLuint seaVBO;
	glGenBuffers(1, &seaVBO);
	glBindBuffer(GL_ARRAY_BUFFER, seaVBO);
	glBufferData(GL_ARRAY_BUFFER, sea.vertices.size() * sizeof(Vertex), &sea.vertices[0], GL_STATIC_DRAW);



	glEnableVertexAttribArray(0);

	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, position) // array buffer offset
	);

	glEnableVertexAttribArray(1);

	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, uv) // array buffer offset
	);

	glEnableVertexAttribArray(2);

	glVertexAttribPointer(
		2, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, normal) // array buffer offset
	);

	GLuint seaEBO;
	glGenBuffers(1, &seaEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, seaEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sea.indices.size() * sizeof(unsigned int), &sea.indices[0], GL_STATIC_DRAW);

	
	std::cout << "sea.indices.size: " << sea.indices.size() << std::endl;

	seaTexture1.setupTexture("resources/sea/sea_01.jpg");
	seaTexture2.setupTexture("resources/sea/sea_02.jpg");

	seaIndicesSize = sea.indices.size();

}

void SendBlockData() {
	Model block = loadOBJ("resources/block/block.obj");
	glGenVertexArrays(1, &blockVAO);
	glBindVertexArray(blockVAO);

	GLuint blockVBO;
	glGenBuffers(1, &blockVBO);
	glBindBuffer(GL_ARRAY_BUFFER, blockVBO);
	glBufferData(GL_ARRAY_BUFFER, block.vertices.size() * sizeof(Vertex), &block.vertices[0], GL_STATIC_DRAW);



	glEnableVertexAttribArray(0);

	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, position) // array buffer offset
	);

	glEnableVertexAttribArray(1);

	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, uv) // array buffer offset
	);

	glEnableVertexAttribArray(2);

	glVertexAttribPointer(
		2, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, normal) // array buffer offset
	);

	GLuint blockEBO;
	glGenBuffers(1, &blockEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, blockEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, block.indices.size() * sizeof(unsigned int), &block.indices[0], GL_STATIC_DRAW);


	std::cout << "block.indices.size: " << block.indices.size() << std::endl;

	blockTexture.setupTexture("resources/block/block_01.bmp");

	blockIndicesSize = block.indices.size();
}


void sendDataToOpenGL()
{
	//TODO
	//Load objects and bind to VAO and VBO
	//Load textures


	//Model dolphin = loadOBJ("resources/dolphin/jeep.obj");


	SendDolphinData();
	SendSeaData();
	SendBlockData();
}

void GLAPIENTRY
MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}

void initializedGL(void) //run only once
{
	if (glewInit() != GLEW_OK) {
		std::cout << "GLEW not OK." << std::endl;
	}

	get_OpenGL_info();
	sendDataToOpenGL();

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);

	//TODO: set up the camera parameters	
	//TODO: set up the vertex shader and fragment shader

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	
	shader.setupShader("VertexShaderCode.glsl","FragmentShaderCode.glsl");
	shader2.setupShader("VertexShaderCode2.glsl", "FragmentShaderCode2.glsl");

	//shader.use();


}

void paintDolphin()
{
	glBindVertexArray(dolphinVAO);
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -100.0f, dolphinZ));
	model = glm::rotate(
		model,
		glm::radians(dolphinRotateY),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	model = glm::rotate(
		model,
		glm::radians(-90.0f),
		glm::vec3(1.0f, 0.0f, 0.0f)
	);
	shader.setMat4("model", model);
	if (dolphinTextureSelect == 1) {
		dolphinTexture1.bind(0);
	}
	if (dolphinTextureSelect == 2) {
		dolphinTexture2.bind(0);
	}

	glDrawElements(GL_TRIANGLES, dolphinIndicesSize, GL_UNSIGNED_INT, nullptr);

}


void paintSea() {
	glBindVertexArray(seaVAO);
	glm::mat4 model = glm::mat4(1.0f);

	model = glm::translate(model, glm::vec3(0.0f, -500.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1500.0f, 1.0f, 1500.0f));

	shader.setMat4("model", model);


	if (seaTextureSelect == 1) {
		seaTexture1.bind(0);
	}
	if (seaTextureSelect == 2) {
		seaTexture2.bind(0);
	}
	glDrawElements(GL_TRIANGLES, seaIndicesSize, GL_UNSIGNED_INT, nullptr);

}

void paintBlock() {
	shader2.use();

	glBindVertexArray(blockVAO);
	glm::mat4 model = glm::mat4(1.0f);


	glm::mat4 view = glm::lookAt(cameraPos, cameraFront, cameraTop);

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);

	shader2.setMat4("view", view);
	shader2.setMat4("projection", projection);


	model = glm::translate(model, glm::vec3(lightX, lightY, lightZ));
	model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));

	shader2.setMat4("model", model);


	//blockTexture.bind(0);
	glDrawElements(GL_TRIANGLES, blockIndicesSize, GL_UNSIGNED_INT, nullptr);


}

void paintBlock2() {
	glBindVertexArray(blockVAO);
	glm::mat4 model = glm::mat4(1.0f);

	model = glm::translate(model, glm::vec3(0.0f + boxX,-400.0f,-1100.0f + boxZ));
	model = glm::scale(model, glm::vec3(50.0f, 50.0f, 50.0f));

	shader.setMat4("model", model);


	blockTexture.bind(0);
	glDrawElements(GL_TRIANGLES, blockIndicesSize, GL_UNSIGNED_INT, nullptr);


}



void paintGL(void)  //always run
{
	glClearColor(0.4f, 0.6f, 1.0f, 1.0f); //background color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//TODO:
	//Set lighting information, such as position and color of lighting source
	//Set transformation matrix
	//Bind different textures


	shader.use();



	glm::mat4 view = glm::lookAt(cameraPos, cameraFront, cameraTop);
	//view = glm::lookAt(cameraPos, cameraPos + glm::vec3( 0.7f, 0.0f, -1.0f), cameraTop);

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);

	shader.setMat4("view", view);
	shader.setMat4("projection", projection);

	//set light
	shader.setVec3("ambientLight", glm::vec3(ambientLightCounter * 0.1f, ambientLightCounter * 0.1f, ambientLightCounter * 0.1f));
	shader.setVec3("diffPos", glm::vec3(lightX,lightY,lightZ));
	shader.setVec3("diffColor", glm::vec3(diffCol, diffCol, diffCol));
	shader.setVec3("specPos", cameraPos);
	shader.setVec3("specColor", glm::vec3(specCol, specCol, specCol));
	shader.setVec3("spotlightPosition", glm::vec3(0.0f + boxX, 200.0f, -1100.0f + boxZ));
	shader.setVec3("spotlightDirection", glm::vec3(0.0f, -1.0f, 0.0f));

	paintDolphin();
	paintSea();
	paintBlock2();

	paintBlock();

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// Sets the mouse-button callback for the current window.	

			
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		IsMouseLeftPress = true;
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		IsMouseLeftPress = false;
		IsFirstMouse = true;
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		IsMouseRightPress = true;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		IsMouseRightPress = false;
		IsFirstMouse = true;

	}
}

void cursor_position_callback(GLFWwindow* window, double x, double y)
{
	// Sets the cursor position callback for the current window

	if (IsMouseLeftPress) {

		if (IsFirstMouse) {
			lastX = x;
			lastY = y;
			IsFirstMouse = false;
		}
		//float xoffset = x - lastX;
		float yoffset = lastY - y; 
		lastX = x;
		lastY = y;

		float sensitivity = 0.1f; 
		//xoffset *= sensitivity;
		yoffset *= sensitivity;

		//yaw += xoffset;
		pitch += yoffset;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		cameraPos = glm::normalize(front) * 500.0f;
	}

	if (IsMouseRightPress) {

		if (IsFirstMouse) {
			lastX = x;
			lastY = y;
			IsFirstMouse = false;
		}
		float xoffset = x - lastX;
		float yoffset = lastY - y; 
		lastX = x;
		lastY = y;

		float sensitivity = 0.1f; 
		xoffset *= sensitivity;
		//yoffset *= sensitivity;

		yaw += xoffset;
		//pitch += yoffset;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		cameraPos = glm::normalize(front) * 500.0f;
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// Sets the scoll callback for the current window.
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Sets the Keyboard callback for the current window.

	//float speed = 1.0f; // adjust accordingly

	if (key == GLFW_KEY_UP && action == GLFW_REPEAT) {
		dolphinZ += 10.0f;
	}
	if (key == GLFW_KEY_DOWN && action == GLFW_REPEAT) {
		dolphinZ -= 10.0f;
	}

	if (key == GLFW_KEY_LEFT && action == GLFW_REPEAT) {
		dolphinRotateY -= 10.0f;
	}

	if (key == GLFW_KEY_RIGHT && action == GLFW_REPEAT) {
		dolphinRotateY += 10.0f;
	}

	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		dolphinTextureSelect = 1;
	}

	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		dolphinTextureSelect = 2;
	}

	if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		seaTextureSelect = 1;
	}
	if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
		seaTextureSelect = 2;
	}

	if (key == GLFW_KEY_E && action == GLFW_REPEAT) {
		lightX += 10.0f;
	}
	if (key == GLFW_KEY_D && action == GLFW_REPEAT) {
		lightX -= 10.0f;
	}

	if (key == GLFW_KEY_R && action == GLFW_REPEAT) {
		lightY += 10.0f;
	}
	if (key == GLFW_KEY_F && action == GLFW_REPEAT) {
		lightY -= 10.0f;
	}

	if (key == GLFW_KEY_T && action == GLFW_REPEAT) {
		lightZ += 10.0f;
	}
	if (key == GLFW_KEY_G && action == GLFW_REPEAT) {
		lightZ -= 10.0f;
	}

	if (key == GLFW_KEY_W && action == GLFW_REPEAT) {
		ambientLightCounter += 1.0f;
	}
	if (key == GLFW_KEY_S && action == GLFW_REPEAT) {
		ambientLightCounter -= 1.0f;
	}

	if (key == GLFW_KEY_Y && action == GLFW_REPEAT) {
		diffCol += 0.1f;
	}
	if (key == GLFW_KEY_H && action == GLFW_REPEAT) {
		diffCol -= 0.1f;
	}

	if (key == GLFW_KEY_U && action == GLFW_REPEAT) {
		specCol += 0.1f;
	}
	if (key == GLFW_KEY_J && action == GLFW_REPEAT) {
		specCol -= 0.1f;
	}

	if (key == GLFW_KEY_7 && action == GLFW_REPEAT) {
		boxX += 100.0f;
	}
	if (key == GLFW_KEY_8 && action == GLFW_REPEAT) {
		boxX -= 100.0f;
	}

	if (key == GLFW_KEY_9 && action == GLFW_REPEAT) {
		boxZ += 100.0f;
	}
	if (key == GLFW_KEY_0 && action == GLFW_REPEAT) {
		boxZ -= 100.0f;
	}
}


int main(int argc, char* argv[])
{
	GLFWwindow* window;

	/* Initialize the glfw */
	if (!glfwInit()) {
		std::cout << "Failed to initialize GLFW" << std::endl;
		return -1;
	}
	/* glfw: configure; necessary for MAC */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment 2", NULL, NULL);
	if (!window) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/*register callback functions*/
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);                                                                  //    
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	initializedGL();

	while (!glfwWindowShouldClose(window)) {
		/* Render here */
		paintGL();

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}






