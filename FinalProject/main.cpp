// Library for OpenGL function loading
// Must be included before GLFW
#define GLEW_STATIC
#include <GL/glew.h>

// Library for window creation and event handling
#include <GLFW/glfw3.h>

// Library for vertex and matrix math
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/normal.hpp>

// Library for loading .OBJ model
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

// Library for loading an image
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Header for camera structure/functions
#include "camera.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>


enum StateType
{
	IDLE = 0,
	ATTACK = 1,
	DEAD = 2,
	STATECOUNT
};

struct MixFactor
{
	float idle = 0.0;
	float attack = 0.0;
	float dead = 0.0;
	float increment = 0.001;
};

double lastFrameTime = 0.0;

// global variables for anivia

struct Character
{
	StateType state = IDLE;
	MixFactor mixFactor;
	float moveSpeed = 0.005;
	float scaleFactor = 1.0;
	glm::vec2 movement = { 0,0 };
	glm::vec3 position = { 0,0,0 };
	GLuint texture;
	GLuint vao, vbo;
	glm::vec3 rotateAxis = { 0,1,0 };
	float rotateAngle = 0.0;

	void move(Camera camera)
	{
		glm::vec3 forward = glm::normalize(camera.forward);
		glm::vec3 up = glm::normalize(camera.up);
		glm::vec3 right = glm::normalize(glm::cross(forward, up));
		glm::vec3 realUp = glm::normalize(glm::cross(right, forward));
		position += right * movement.x + realUp * movement.y;
	}
	
	void updateMixFactor()
	{
		if (state == IDLE)
		{
			if (mixFactor.attack >= 0)
				mixFactor.attack -= abs(mixFactor.increment);
			if (mixFactor.dead >= 0)
				mixFactor.dead -= abs(mixFactor.increment);

			if (mixFactor.idle > 1.0)
				mixFactor.increment = -abs(mixFactor.increment);
			else if (mixFactor.idle < 0.0)
				mixFactor.increment = abs(mixFactor.increment);
			mixFactor.idle += mixFactor.increment;
		}
		else if (state == ATTACK)
		{
			if (mixFactor.attack <= 1.0)
				mixFactor.attack += abs(mixFactor.increment);
		}
		else if (state == DEAD)
		{
			if (mixFactor.dead <= 1.0)
				mixFactor.dead += abs(mixFactor.increment);
		}
	}
};

Character anivia;
Character enemy;


// Configuration
const int WIDTH = 800;
const int HEIGHT = 600;


struct Mouse
{
	glm::vec2 screenCoor;// screen coordinates (left,bottom)(0,0) -> (right,top)(1,1)
	void updateScreenCoor(double xpos, double ypos)
	{
		screenCoor.x = xpos / WIDTH;
		screenCoor.y = (HEIGHT - ypos) / HEIGHT;
	}

};

Mouse mouse;


// Per-vertex data
struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texCoor;
};
struct EnemyVertex :Vertex {
	glm::vec3 pos_idle;
	glm::vec3 normal_idle;
};
struct AniviaVertex:Vertex
{
	glm::vec3 pos_idle;
	glm::vec3 normal_idle;
	glm::vec3 pos_attack;
	glm::vec3 normal_attack;
	glm::vec3 pos_dead;
	glm::vec3 normal_dead;
};


struct Terrain
{
	int NbVertX, NbVertY;
	glm::vec3 position = { -5.0,-5.0,5.0 };
	glm::vec3 rotateAxis = { 1.0,0.0,0.0 };
	float rotateAngle = 3.14159/2;
	int startingRow = 0; //draw from this row
	std::vector <std::vector<Vertex>> grid;
	float updateInterval = 1.0;
	std::vector <Vertex> surface;
	GLuint texture;
	double lastUpdateTime = 0;
	double lastFrameTime = 0;
	void generateTerrain()
	{
		// i - row; j - column
		for (int i = 0; i < NbVertY; i++)
		{
			std::vector<Vertex> row;
			for (int j = 0; j < NbVertX; j++)
			{
				Vertex vertex;
				float height;
				height = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				vertex.pos = { j, i , height };
				vertex.texCoor = {j / float(NbVertX), i / float(NbVertY) };
				row.push_back(vertex);
			}
			grid.push_back(row);
		}
		generateTriangles();
	}
	void generateTriangles() 
	{
		for (int i = 0; i < NbVertY; i++)
		{
			for (int j = 0; j < NbVertX-1; j++)
			{
				Vertex vertex_1, vertex_2, vertex_3, vertex_4;
				vertex_1 = grid[i][j];
				vertex_2 = grid[i][j + 1];

				if (i == NbVertY-1)
				{
					vertex_3 = grid[0][j + 1];
					vertex_4 = grid[0][j];
					vertex_3.pos.y += NbVertY;
					vertex_4.pos.y += NbVertY;
				}
				else
				{
					vertex_3 = grid[(i + 1)][j + 1];
					vertex_4 = grid[(i + 1)][j];
				}
				
				glm::vec3 normal_1, normal_2;
				normal_1 = glm::triangleNormal(vertex_1.pos, vertex_2.pos, vertex_3.pos);
				normal_2 = glm::triangleNormal(vertex_3.pos, vertex_4.pos, vertex_1.pos);

				vertex_1.normal = normal_1;
				vertex_2.normal = normal_1;
				vertex_3.normal = normal_1;
				surface.push_back(vertex_1); 
				surface.push_back(vertex_2); 
				surface.push_back(vertex_3);
				
				vertex_3.normal = normal_2;
				vertex_4.normal = normal_2;
				vertex_1.normal = normal_2;
				surface.push_back(vertex_3);
				surface.push_back(vertex_4);
				surface.push_back(vertex_1);				
			}
		}
	}
	void update()
	{
		double currentTime = glfwGetTime();
		position.z += (currentTime - lastFrameTime) / updateInterval;
		lastFrameTime = currentTime;

		if (currentTime - lastUpdateTime < updateInterval)
			return;
		lastUpdateTime = currentTime;
		int startingIndex = 2 * 3 * (NbVertX - 1) * startingRow;
		for (int i = 0; i < 2 * 3 * (NbVertX-1); i++)
		{
			surface[startingIndex + i].pos.y += NbVertY;

		}
		startingRow++;
		startingRow %= NbVertY;
	}
};

// Helper function to read a file like a shader
std::string readFile(const std::string& path) {
	std::ifstream file(path, std::ios::binary);
	
	std::stringstream buffer;
	buffer << file.rdbuf();

	return buffer.str();
}

bool checkShaderErrors(GLuint shader) {
	// Check if the shader compiled successfully
	GLint compileSuccessful;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileSuccessful);

	// If it didn't, then read and print the compile log
	if (!compileSuccessful) {
		GLint logLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

		std::vector<GLchar> logBuffer(logLength);
		glGetShaderInfoLog(shader, logLength, nullptr, logBuffer.data());

		std::cerr << logBuffer.data() << std::endl;
		
		return false;
	} else {
		return true;
	}
}

bool checkProgramErrors(GLuint program) {
	// Check if the program linked successfully
	GLint linkSuccessful;
	glGetProgramiv(program, GL_LINK_STATUS, &linkSuccessful);

	// If it didn't, then read and print the link log
	if (!linkSuccessful) {
		GLint logLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

		std::vector<GLchar> logBuffer(logLength);
		glGetProgramInfoLog(program, logLength, nullptr, logBuffer.data());

		std::cerr << logBuffer.data() << std::endl;
		
		return false;
	} else {
		return true;
	}
}

// OpenGL debug callback
void APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) {
		std::cerr << "OpenGL: " << message << std::endl;
	}
}


// Key handle function
void keyboardHandler(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	cameraKeyboardHandler(key, action);

	float moveSpeed = anivia.moveSpeed;
	glm::vec2 movement = { 0,0 };

	switch (key) 
	{
	case GLFW_KEY_1:
		anivia.state = IDLE;
		break;
	case GLFW_KEY_2:
		anivia.state = ATTACK;
		break;
	case GLFW_KEY_UP:
		if (action == GLFW_PRESS) movement.y = moveSpeed;
		if (action == GLFW_RELEASE) movement.y = 0.0;
		break;
	case GLFW_KEY_DOWN:
		if (action == GLFW_PRESS)movement.y = -moveSpeed;
		if (action == GLFW_RELEASE)movement.y = 0.0;
		break;
	case GLFW_KEY_LEFT:
		if (action == GLFW_PRESS)movement.x = -moveSpeed;
		if (action == GLFW_RELEASE)movement.x = 0.0;
		break;
	case GLFW_KEY_RIGHT:
		if (action == GLFW_PRESS)movement.x = moveSpeed;
		if (action == GLFW_RELEASE)movement.x = 0.0;
		break;
	default:
		break;
	}

	anivia.movement = movement;
}

glm::vec3 updatePosition(glm::vec3 position, glm::vec2 moveSpeed, Camera camera)
{
	glm::vec3 forward = glm::normalize(camera.forward);
	glm::vec3 up = glm::normalize(camera.up);
	glm::vec3 right = glm::normalize(glm::cross(forward, up));
	glm::vec3 realUp = glm::normalize(glm::cross(right, forward));
	position += realUp * moveSpeed.y + right * moveSpeed.x;
	return position;
}


// Mouse button handle function
void mouseButtonHandler(GLFWwindow* window, int button, int action, int mods)
{
	camMouseButtonHandler(button, action);
}

void cursorPosHandler(GLFWwindow* window, double xpos, double ypos)
{
	camCursorPosHandler(xpos, ypos);
	mouse.updateScreenCoor(xpos, ypos);
	//std::cerr << xpos << '\t' << ypos << std::endl;
}

//declaration
void updateMixFactor(StateType state, MixFactor &mixFactor);
glm::vec3 updatePosition(glm::vec3 position, glm::vec2 moveSpeed, Camera camera);


int main() {
	//definitions
	Terrain terrain;
	terrain.NbVertX = 10;
	terrain.NbVertY = 10;
	terrain.generateTerrain();

	terrain.lastFrameTime = glfwGetTime();


	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW!" << std::endl;
		return EXIT_FAILURE;
	}

	//////////////////// Create window and OpenGL 4.3 debug context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Shadow mapping practical", nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create OpenGL context!" << std::endl;
		std::cout <<  "Press enter to close."; getchar();
		return EXIT_FAILURE;
	}

	glfwSetKeyCallback(window, keyboardHandler);
	glfwSetMouseButtonCallback(window, mouseButtonHandler);
	glfwSetCursorPosCallback(window, cursorPosHandler);

	// Activate the OpenGL context
	glfwMakeContextCurrent(window);

	// Initialize GLEW extension loader
	glewExperimental = GL_TRUE;
	glewInit();

	// Set up OpenGL debug callback
	glDebugMessageCallback(debugCallback, nullptr);

	GLuint mainProgram = glCreateProgram();
	GLuint shadowProgram = glCreateProgram();
	GLuint terrainProgram = glCreateProgram();


	////////////////// Load and compile main shader program
	{
		std::string vertexShaderCode = readFile("shader.vert");
		const char* vertexShaderCodePtr = vertexShaderCode.data();

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderCodePtr, nullptr);
		glCompileShader(vertexShader);

		std::string fragmentShaderCode = readFile("shader.frag");
		const char* fragmentShaderCodePtr = fragmentShaderCode.data();

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderCodePtr, nullptr);
		glCompileShader(fragmentShader);

		if (!checkShaderErrors(vertexShader) || !checkShaderErrors(fragmentShader)) {
			std::cerr << "Shader(s) failed to compile!" << std::endl;
			std::cout << "Press enter to close."; getchar();
			return EXIT_FAILURE;
		}

		// Combine vertex and fragment shaders into single shader program
		glAttachShader(mainProgram, vertexShader);
		glAttachShader(mainProgram, fragmentShader);
		glLinkProgram(mainProgram);

		if (!checkProgramErrors(mainProgram)) {
			std::cerr << "Main program failed to link!" << std::endl;
			std::cout << "Press enter to close."; getchar();
			return EXIT_FAILURE;
		}
	}

	////////////////// Load and compile shadow shader program
	{
		std::string vertexShaderCode = readFile("shadow.vert");
		const char* vertexShaderCodePtr = vertexShaderCode.data();

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderCodePtr, nullptr);
		glCompileShader(vertexShader);

		std::string fragmentShaderCode = readFile("shadow.frag");
		const char* fragmentShaderCodePtr = fragmentShaderCode.data();

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderCodePtr, nullptr);
		glCompileShader(fragmentShader);

		if (!checkShaderErrors(vertexShader) || !checkShaderErrors(fragmentShader)) {
			std::cerr << "Shader(s) failed to compile!" << std::endl;
			return EXIT_FAILURE;
		}

		// Combine vertex and fragment shaders into single shader program
		glAttachShader(shadowProgram, vertexShader);
		glAttachShader(shadowProgram, fragmentShader);
		glLinkProgram(shadowProgram);

		if (!checkProgramErrors(shadowProgram)) {
			std::cerr << "Shadow program failed to link!" << std::endl;
			return EXIT_FAILURE;
		}
	}

	////////////////// Load and compile terrain shader program
	{
		std::string vertexShaderCode = readFile("terrain.vert");
		const char* vertexShaderCodePtr = vertexShaderCode.data();

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderCodePtr, nullptr);
		glCompileShader(vertexShader);

		std::string fragmentShaderCode = readFile("terrain.frag");
		const char* fragmentShaderCodePtr = fragmentShaderCode.data();

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderCodePtr, nullptr);
		glCompileShader(fragmentShader);

		if (!checkShaderErrors(vertexShader) || !checkShaderErrors(fragmentShader)) {
			std::cerr << "Shader(s) failed to compile!" << std::endl;
			std::cout << "Press enter to close."; getchar();
			return EXIT_FAILURE;
		}

		// Combine vertex and fragment shaders into single shader program
		glAttachShader(terrainProgram, vertexShader);
		glAttachShader(terrainProgram, fragmentShader);
		glLinkProgram(terrainProgram);

		if (!checkProgramErrors(terrainProgram)) {
			std::cerr << "Main program failed to link!" << std::endl;
			std::cout << "Press enter to close."; getchar();
			return EXIT_FAILURE;
		}
	}
	//////////////////// Load a texture for exercise 5
	// Create Texture
	//int texwidth, texheight, texchannels;
	//stbi_uc* pixels = stbi_load("smiley.png", &texwidth, &texheight, &texchannels, 3);
	//
	//GLuint texLight;
	//glGenTextures(1, &texLight);
	//glBindTexture(GL_TEXTURE_2D, texLight);

	//// Upload pixels into texture
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texwidth, texheight, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	//// Set behaviour for when texture coordinates are outside the [0, 1] range
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//// Set interpolation for texture sampling (GL_NEAREST for no interpolation)
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	////////////////////////// Load vertices of model
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	//defination of position (offset)
	glm::vec3 aniviaPosition = { 0.0, 0.0, 0.0 };

	//defination of vertices
	std::vector<Vertex> vertices;
	std::vector<AniviaVertex> aniviaVertices;
	std::vector<EnemyVertex> enemyVertices;
	std::vector<AniviaVertex> aniviaHeadVertices;


	//load ANIVIA
	{
		int vertexCounter = 0;
		// load initial pose
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "anivia_start.obj")) {
			std::cerr << err << std::endl;
			return EXIT_FAILURE;
		}
		// Read triangle vertices from OBJ file
		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				AniviaVertex vertex = {};

				// Retrieve coordinates for vertex by index
				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				// Retrieve components of normal by index
				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};

				// Retrieve coordinates for texture
				vertex.texCoor = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					attrib.texcoords[2 * index.texcoord_index + 1]
				};
				
				aniviaVertices.push_back(vertex);
			}
		}

		//load idle pose (animation between initial and idle pose)
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "anivia_open_wing.obj")) {
			std::cerr << err << std::endl;
			return EXIT_FAILURE;
		}
		// Read triangle vertices from OBJ file
		vertexCounter = 0;
		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex = {};

				// Retrieve coordinates for vertex by index
				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				// Retrieve components of normal by index
				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};

				aniviaVertices[vertexCounter].pos_idle = vertex.pos;
				aniviaVertices[vertexCounter].normal_idle = vertex.normal;
				vertexCounter++;
			}
		}

		//load attack pose
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "anivia_attack.obj")) {
			std::cerr << err << std::endl;
			return EXIT_FAILURE;
		}
		// Read triangle vertices from OBJ file
		vertexCounter = 0;
		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex = {};

				// Retrieve coordinates for vertex by index
				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				// Retrieve components of normal by index
				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};

				aniviaVertices[vertexCounter].pos_attack = vertex.pos;
				aniviaVertices[vertexCounter].normal_attack = vertex.normal;
				vertexCounter++;
			}
		}

		//load idle pose (animation between initial and idle pose)
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "anivia_dead.obj")) {
			std::cerr << err << std::endl;
			return EXIT_FAILURE;
		}
		// Read triangle vertices from OBJ file
		vertexCounter = 0;
		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex = {};

				// Retrieve coordinates for vertex by index
				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				// Retrieve components of normal by index
				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};

				aniviaVertices[vertexCounter].pos_dead = vertex.pos;
				aniviaVertices[vertexCounter].pos_dead = vertex.normal;
				vertexCounter++;
			}
		}


	}

	// load texture for anivia
	int width, height, channels;
	stbi_uc* pixels = stbi_load("anivia.png", &width, &height, &channels, 3);

	// Create Texture

	glGenTextures(1, &anivia.texture);
	glBindTexture(GL_TEXTURE_2D, anivia.texture);

	// Upload pixels into texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	// Set behaviour for when texture coordinates are outside the [0, 1] range
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Set interpolation for texture sampling (GL_NEAREST for no interpolation)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	/////// handle the vertices of anivia
	{
		glGenBuffers(1, &anivia.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, anivia.vbo);
		glBufferData(GL_ARRAY_BUFFER, aniviaVertices.size() * sizeof(AniviaVertex), aniviaVertices.data(), GL_STATIC_DRAW);

		glGenVertexArrays(1, &anivia.vao);
		glBindVertexArray(anivia.vao);

		glBindBuffer(GL_ARRAY_BUFFER, anivia.vbo);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(AniviaVertex), reinterpret_cast<void*>(offsetof(AniviaVertex, pos)));
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, anivia.vbo);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(AniviaVertex), reinterpret_cast<void*>(offsetof(AniviaVertex, normal)));
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, anivia.vbo);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(AniviaVertex), reinterpret_cast<void*>(offsetof(AniviaVertex, pos_idle)));
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, anivia.vbo);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(AniviaVertex), reinterpret_cast<void*>(offsetof(AniviaVertex, normal_idle)));
		glEnableVertexAttribArray(3);

		glBindBuffer(GL_ARRAY_BUFFER, anivia.vbo);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(AniviaVertex), reinterpret_cast<void*>(offsetof(AniviaVertex, pos_attack)));
		glEnableVertexAttribArray(4);

		glBindBuffer(GL_ARRAY_BUFFER, anivia.vbo);
		glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(AniviaVertex), reinterpret_cast<void*>(offsetof(AniviaVertex, normal_attack)));
		glEnableVertexAttribArray(5);

		glBindBuffer(GL_ARRAY_BUFFER, anivia.vbo);
		glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(AniviaVertex), reinterpret_cast<void*>(offsetof(AniviaVertex, pos_dead)));
		glEnableVertexAttribArray(6);

		glBindBuffer(GL_ARRAY_BUFFER, anivia.vbo);
		glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(AniviaVertex), reinterpret_cast<void*>(offsetof(AniviaVertex, normal_dead)));
		glEnableVertexAttribArray(7);
		
		glBindBuffer(GL_ARRAY_BUFFER, anivia.vbo);
		glVertexAttribPointer(8, 2, GL_FLOAT, GL_FALSE, sizeof(AniviaVertex), reinterpret_cast<void*>(offsetof(AniviaVertex, texCoor)));
		glEnableVertexAttribArray(8);
	}
	


	//////////////////init for enemy
	enemy.position = { 0,0,3 };
	enemy.scaleFactor = 0.3;
	enemy.rotateAngle = 3.14159;
	enemy.movement.y = -0.0005;
	//load
	{
		int vertexCounter = 0;
		// load initial pose
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "aatrox_low.obj")) {
			std::cerr << err << std::endl;
			return EXIT_FAILURE;
		}
		// Read triangle vertices from OBJ file
		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				EnemyVertex vertex = {};

				// Retrieve coordinates for vertex by index
				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				// Retrieve components of normal by index
				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};

				// Retrieve coordinates for texture
				vertex.texCoor = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					attrib.texcoords[2 * index.texcoord_index + 1]
				};

				enemyVertices.push_back(vertex);
			}
		}

		//load idle pose (animation between initial and idle pose)
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "aatrox_high.obj")) {
			std::cerr << err << std::endl;
			return EXIT_FAILURE;
		}
		// Read triangle vertices from OBJ file
		vertexCounter = 0;
		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex = {};

				// Retrieve coordinates for vertex by index
				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				// Retrieve components of normal by index
				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};

				enemyVertices[vertexCounter].pos_idle = vertex.pos;
				enemyVertices[vertexCounter].normal_idle = vertex.normal;
				vertexCounter++;
			}
		}
		
	}

	// load texture for enemy
	pixels = stbi_load("Aatrox_Base_Mat.png", &width, &height, &channels, 3);

	// Create Texture

	glGenTextures(1, &enemy.texture);
	glBindTexture(GL_TEXTURE_2D, enemy.texture);

	// Upload pixels into texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	// Set behaviour for when texture coordinates are outside the [0, 1] range
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Set interpolation for texture sampling (GL_NEAREST for no interpolation)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	/////// handle the vertices of enemy
	{
		glGenBuffers(1, &enemy.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, enemy.vbo);
		glBufferData(GL_ARRAY_BUFFER, enemyVertices.size() * sizeof(EnemyVertex), enemyVertices.data(), GL_STATIC_DRAW);

		glGenVertexArrays(1, &enemy.vao);
		glBindVertexArray(enemy.vao);

		glBindBuffer(GL_ARRAY_BUFFER, enemy.vbo);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(EnemyVertex), reinterpret_cast<void*>(offsetof(EnemyVertex, pos)));
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, enemy.vbo);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(EnemyVertex), reinterpret_cast<void*>(offsetof(EnemyVertex, normal)));
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, enemy.vbo);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(EnemyVertex), reinterpret_cast<void*>(offsetof(EnemyVertex, pos_idle)));
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, enemy.vbo);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(EnemyVertex), reinterpret_cast<void*>(offsetof(EnemyVertex, normal_idle)));
		glEnableVertexAttribArray(3);

		glBindBuffer(GL_ARRAY_BUFFER, enemy.vbo);
		glVertexAttribPointer(8, 2, GL_FLOAT, GL_FALSE, sizeof(EnemyVertex), reinterpret_cast<void*>(offsetof(EnemyVertex, texCoor)));
		glEnableVertexAttribArray(8);
	}

	//////////////////// Create Vertex Buffer Object
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	// Bind vertex data to shader inputs using their index (location)
	// These bindings are stored in the Vertex Array Object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// The position vectors should be retrieved from the specified Vertex Buffer Object with given offset and stride
	// Stride is the distance in bytes between vertices
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, pos)));
	glEnableVertexAttribArray(0);

	// The normals should be retrieved from the same Vertex Buffer Object (glBindBuffer is optional)
	// The offset is different and the data should go to input 1 instead of 0
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));
	glEnableVertexAttribArray(1);


	////////////////terrain
	GLuint vbo_terrain, vao_terrain;
	{
		glGenBuffers(1, &vbo_terrain);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_terrain);
		glBufferData(GL_ARRAY_BUFFER, terrain.surface.size() * sizeof(EnemyVertex), terrain.surface.data(), GL_STATIC_DRAW);

		glGenVertexArrays(1, &vao_terrain);
		glBindVertexArray(vao_terrain);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_terrain);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, pos)));
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_terrain);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_terrain);
		glVertexAttribPointer(8, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, texCoor)));
		glEnableVertexAttribArray(8);
	}
	// add texture for terrain
	pixels = stbi_load("terrain.png", &width, &height, &channels, 3);

	// Create Texture

	glGenTextures(1, &terrain.texture);
	glBindTexture(GL_TEXTURE_2D, terrain.texture);

	// Upload pixels into texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	// Set behaviour for when texture coordinates are outside the [0, 1] range
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Set interpolation for texture sampling (GL_NEAREST for no interpolation)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//////////////////// Create Shadow Texture
	GLuint texShadow;
	const int SHADOWTEX_WIDTH  = 1024;
	const int SHADOWTEX_HEIGHT = 1024;
	glGenTextures(1, &texShadow);
	glBindTexture(GL_TEXTURE_2D, texShadow);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, SHADOWTEX_WIDTH, SHADOWTEX_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	// Set behaviour for when texture coordinates are outside the [0, 1] range
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Set interpolation for texture sampling (GL_NEAREST for no interpolation)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//////////////////// Create framebuffer for extra texture
	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);

	/////////////////// Set shadow texure as depth buffer for this framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texShadow, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/////////////////// Create main camera
	Camera mainCamera;
	mainCamera.aspect = WIDTH / (float)HEIGHT;
	mainCamera.position = glm::vec3(0.0f, 10.0f, 0.0f);
	//mainCamera.forward  = -mainCamera.position;
	mainCamera.forward = glm::vec3(0.0f, -1.0f, -0.0f);

	// Main loop
	while (!glfwWindowShouldClose(window)) {

		//update 
		anivia.move(mainCamera);
		anivia.updateMixFactor();
		
		enemy.move(mainCamera);
		enemy.updateMixFactor();

		terrain.update();

		glm::vec4 tmp = mainCamera.vpMatrix()*glm::vec4(anivia.position, 1.0);
		glm::vec2 screenCoor = { tmp.x / tmp.w, tmp.y / tmp.w };
		screenCoor = screenCoor * 0.5f + glm::vec2(0.5, 0.5);

		anivia.rotateAxis = { 0.0, 1.0, 0.0 };
		double angle;
		
		angle = glm::orientedAngle(glm::vec2(0.0, 1.0), glm::normalize(mouse.screenCoor - screenCoor));
		//std::cerr << angle << std::endl;
		//anivia.rotateAngle = -angle;

		
		
		//std::cerr << tmp.x / tmp.w << '\t' << tmp.y / tmp.w << '\t' << tmp.z / tmp.w << '\t' << tmp.w / tmp.w << std::endl;
		//std::cerr << screenCoor.x << screenCoor.y << std::endl;

		glfwPollEvents();

		////////// Stub code for you to fill in order to render the shadow map
		{
			// Bind the off-screen framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
			
			// Clear the shadow map and set needed options
			glClearDepth(1.0f);
			glClear(GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);

			// Bind the shader
			glUseProgram(shadowProgram);

			// Set viewport size
			glViewport(0, 0, SHADOWTEX_WIDTH, SHADOWTEX_HEIGHT);

			// Execute draw command
			glDrawArrays(GL_TRIANGLES, 0, vertices.size());


			// .... HERE YOU MUST ADD THE CORRECT UNIFORMS FOR RENDERING THE SHADOW MAP

			// Bind vertex data
			glBindVertexArray(vao);

			// Execute draw command
			glDrawArrays(GL_TRIANGLES, 0, vertices.size());

			// Unbind the off-screen framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		// Bind the shader
		glUseProgram(mainProgram); 

		updateCamera(mainCamera);
		
		glm::mat4 mvp = mainCamera.vpMatrix();

		glUniformMatrix4fv(glGetUniformLocation(mainProgram, "mvp"), 1, GL_FALSE, glm::value_ptr(mvp));

		// Set view position
		glUniform3fv(glGetUniformLocation(mainProgram, "viewPos"), 1, glm::value_ptr(mainCamera.position));

		// Expose current time in shader uniform
		glUniform1f(glGetUniformLocation(mainProgram, "time"), static_cast<float>(glfwGetTime()));
		

		glUniform1f(glGetUniformLocation(mainProgram, "mixFactor_idle"), anivia.mixFactor.idle);
		glUniform1f(glGetUniformLocation(mainProgram, "mixFactor_attack"), anivia.mixFactor.attack);
		glUniform1f(glGetUniformLocation(mainProgram, "mixFactor_death"), anivia.mixFactor.dead);
		glUniform3fv(glGetUniformLocation(mainProgram, "pos_offset"), 1, glm::value_ptr(anivia.position));
		glUniform3fv(glGetUniformLocation(mainProgram, "rotateAxis"), 1, glm::value_ptr(anivia.rotateAxis));
		glUniform1f(glGetUniformLocation(mainProgram, "rotateAngle"), anivia.rotateAngle);
		glUniform1f(glGetUniformLocation(mainProgram, "scaleFactor"), anivia.scaleFactor);


		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, anivia.texture);
		glUniform1i(glGetUniformLocation(mainProgram, "tex"), 1);

		// Bind vertex data
		glBindVertexArray(anivia.vao);

		// Bind the shadow map to texture slot 0
		GLint texture_unit = 0;
		glActiveTexture(GL_TEXTURE0 + texture_unit);
		glBindTexture(GL_TEXTURE_2D, texShadow);
		glUniform1i(glGetUniformLocation(mainProgram, "texShadow"), texture_unit);

		// Set viewport size
		glViewport(0, 0, WIDTH, HEIGHT);

		// Clear the framebuffer to black and depth to maximum value
		glClearDepth(1.0f);  
		glClearColor(0.1f, 0.2f, 0.3f, 1.0f); 
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

		// Execute draw command
		glDrawArrays(GL_TRIANGLES, 0, aniviaVertices.size());

		glBindVertexArray(enemy.vao); 
		glUniform1f(glGetUniformLocation(mainProgram, "mixFactor_idle"), enemy.mixFactor.idle);
		glUniform1f(glGetUniformLocation(mainProgram, "mixFactor_attack"), enemy.mixFactor.attack);
		glUniform1f(glGetUniformLocation(mainProgram, "mixFactor_death"), enemy.mixFactor.dead);
		glUniform3fv(glGetUniformLocation(mainProgram, "pos_offset"), 1, glm::value_ptr(enemy.position));
		glUniform3fv(glGetUniformLocation(mainProgram, "rotateAxis"), 1, glm::value_ptr(enemy.rotateAxis));
		glUniform1f(glGetUniformLocation(mainProgram, "rotateAngle"), enemy.rotateAngle);
		glUniform1f(glGetUniformLocation(mainProgram, "scaleFactor"), enemy.scaleFactor);


		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, enemy.texture);
		glUniform1i(glGetUniformLocation(mainProgram, "tex"), 1);
		glDrawArrays(GL_TRIANGLES, 0, enemyVertices.size());


		{
			
			glBindBuffer(GL_ARRAY_BUFFER, vbo_terrain);
			glBufferSubData(GL_ARRAY_BUFFER, 0, terrain.surface.size() * sizeof(Vertex), terrain.surface.data());
			glBindVertexArray(vao_terrain);

			//glBufferData(GL_ARRAY_BUFFER, aniviaVertices.size() * sizeof(AniviaVertex), aniviaVertices.data(), GL_STATIC_DRAW);


			glBindBuffer(GL_ARRAY_BUFFER, vbo_terrain);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, pos)));
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, vbo_terrain);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));
			glEnableVertexAttribArray(1);

		}

		glBindVertexArray(vao_terrain);
		glUniform1f(glGetUniformLocation(mainProgram, "mixFactor_idle"), 0.0);
		glUniform1f(glGetUniformLocation(mainProgram, "mixFactor_attack"), 0.0);
		glUniform1f(glGetUniformLocation(mainProgram, "mixFactor_death"), 0.0);
		glUniform3fv(glGetUniformLocation(mainProgram, "pos_offset"), 1, glm::value_ptr(terrain.position));
		glUniform3fv(glGetUniformLocation(mainProgram, "rotateAxis"), 1, glm::value_ptr(terrain.rotateAxis));
		glUniform1f(glGetUniformLocation(mainProgram, "rotateAngle"), terrain.rotateAngle);
		glUniform1f(glGetUniformLocation(mainProgram, "scaleFactor"), 1.0);
		glActiveTexture(GL_TEXTURE0 + 3);
		glBindTexture(GL_TEXTURE_2D, terrain.texture);
		glUniform1i(glGetUniformLocation(mainProgram, "tex"), 3);
		glDrawArrays(GL_TRIANGLES, 0, terrain.surface.size());

		// Present result to the screen
		glfwSwapBuffers(window);

		lastFrameTime = glfwGetTime();
	}

	glDeleteFramebuffers(1, &framebuffer);

	glDeleteTextures(1, &texShadow);

	glfwDestroyWindow(window);
	
	glfwTerminate();

    return 0;
}
//
//void updateMixFactor(StateType state, MixFactor &mixFactor)
//{
//	if (state == IDLE)
//	{
//		if (mixFactor.attack >= 0)
//			mixFactor.attack -= abs(mixFactor.increment);
//		if (mixFactor.dead >= 0)
//			mixFactor.dead -= abs(mixFactor.increment);
//
//		if (mixFactor.idle > 1.0)
//			mixFactor.increment = -abs(mixFactor.increment);
//		else if (mixFactor.idle < 0.0)
//			mixFactor.increment = abs(mixFactor.increment);
//		mixFactor.idle += mixFactor.increment;
//	}
//	else if (state == ATTACK)
//	{
//		if (mixFactor.attack <= 1.0)
//			mixFactor.attack += abs(mixFactor.increment);
//	}
//	else if (state == DEAD)
//	{
//		if (mixFactor.dead <= 1.0)
//			mixFactor.dead += abs(mixFactor.increment);
//	}
//}
