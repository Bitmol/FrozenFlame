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

#include "Model.h"
#include "Vec3D.h"
#include "mesh.h"
#include "grid.h"


Mesh mesh;
Mesh simplified;
Grid grid;

bool lightView = false;

double lastFrameTime = 0.0;
double const maxFrameRate = 60;

// global variables

glm::vec3 lightDir = { 0,-1,1 };
int Model::textureCount = 1;

Anivia anivia;
//Enemy enemy;
Boss boss;
std::vector<Enemy> enemies;
IceBerg iceBerg;

Shape flame, icicleDiamond;
std::vector<Shape> icicles;
std::vector<Shape> flames;
std::vector<Shape> lifeCrystals;
int currentIcicle = 0;
int currentFlame = 0;
bool bossHit = false;

//zoom camera parameters
float camX = 0.0;
float camY = 12.0;
float camZ = 0.0;

Terrain terrain(20, 20, lightDir);


// Configuration
const int WIDTH = 600;
const int HEIGHT = 800;

std::vector<BossVertex> formatMeshVertices(std::vector<Vertex> vertices, std::vector<Triangle> triangles)
{
	std::vector<BossVertex> bossVertices;
	for (int i=0;i<triangles.size();++i)
	{
	    for(int v = 0; v < 3 ; v++){
			BossVertex vertex;
			glm::vec3 pos, normal;
			normal = { vertices[triangles[i].v[v]].n[0], vertices[triangles[i].v[v]].n[1], vertices[triangles[i].v[v]].n[2] };
			pos = { vertices[triangles[i].v[v]].p[0], vertices[triangles[i].v[v]].p[1] , vertices[triangles[i].v[v]].p[2] };
			vertex.pos = pos;
			vertex.normal = normal;
			bossVertices.push_back(vertex);
	    }
	
	}
	return bossVertices;
}

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
	glm::vec3 movement = { 0,0,0 };

	switch (key) 
	{
	case GLFW_KEY_1:
		boss.state = DAMAGE1;
		break;
	case GLFW_KEY_2:
		if (action == GLFW_PRESS) lightView = !lightView;
		break;
	case GLFW_KEY_3:
		boss.state = DAMAGE2;
		break;
	case GLFW_KEY_W:
		if (action == GLFW_PRESS || action==GLFW_REPEAT) movement.y = moveSpeed;
		if (action == GLFW_RELEASE) movement.y = 0.0;
		break;
	case GLFW_KEY_S:
		if (action == GLFW_PRESS || action == GLFW_REPEAT)movement.y = -moveSpeed;
		if (action == GLFW_RELEASE)movement.y = 0.0;
		break;
	case GLFW_KEY_A:
		if (action == GLFW_PRESS || action == GLFW_REPEAT)movement.x = -moveSpeed;
		if (action == GLFW_RELEASE)movement.x = 0.0;
		break;
	case GLFW_KEY_D:
		if (action == GLFW_PRESS || action == GLFW_REPEAT)movement.x = moveSpeed;
		if (action == GLFW_RELEASE)movement.x = 0.0;
		break;
	case GLFW_KEY_ESCAPE:
		exit(0);
		break;
	case GLFW_KEY_SPACE:
		if (action == GLFW_PRESS)
		{
			if (anivia.state == IDLE && anivia.coolDownCounter <= 0)
			{
				icicles[currentIcicle].state = TRIGGERED;

			}
			
		}
		if (action == GLFW_RELEASE)
		{
			if (anivia.state == IDLE && anivia.coolDownCounter <= 0)
			{

				anivia.state = ATTACK;
				anivia.coolDownCounter = anivia.coolDownTime;
				icicles[currentIcicle].state = SHOT;
				currentIcicle++;
				currentIcicle %= icicles.size();
				icicles[currentIcicle].state = LOADING;

			}
			
		}
		break;
	default:
		break;
	}

	anivia.movement = movement;
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

void initAnivia(Anivia &anivia)
{
	anivia.position.z = -3;
	anivia.coolDownTime = 1.0;
	anivia.mixFactor.increment = 0.05;
	anivia.safeDistance = 2.0;
}

void initBoss(Boss &boss)
{
	boss.state = IDLE;
	boss.position = { 0,1,2.2 };
	boss.scaleFactor = 3;
	boss.rotateAxis = { 0,1,0 };
	boss.rotateAngle = 3.14159;
	boss.safeDistance = 2.0;
	boss.coolDownTime = 3.0;
	boss.mixFactor.increment = 0.05;
	mesh.loadMesh("boss.obj");
	boss.vertices = formatMeshVertices(mesh.vertices, mesh.triangles);
	boss.simplifiedVertices.push_back(boss.vertices);
		
	for (int i = 0; i < 5; i++)
	{
		Mesh simplified;
		simplified = grid.simplifyMesh(mesh, 70 - i * 10);
		for (int y = 0; y < simplified.vertices.size(); y++) {
			simplified.vertices[y].p[1] += 0.50;
			simplified.vertices[y].p[2] -= 0.17;
		}

		boss.simplifiedVertices.push_back(formatMeshVertices(simplified.vertices, simplified.triangles));
	}	
}

void initIcicles(std::vector<Shape> &icicles)
{
	Shape shape;
	const int vertexNumber = 5;
	shape.moveSpeed = 5;
	shape.scaleFactor = 0;
	shape.rotateAxis = { 0,1,0 };
	shape.state = WAITING;
	shape.offset = { 0,0,1.5 };
	float vertices[vertexNumber][3] =
	{
		0, 0, 1.0,//Vertex 0
		-0.2, 0, 0, // vertex 1
		-0.1, 0, -0.2,  // Vertex 2
		0.1, 0, -0.2, // Vertex3
		0.2, 0, 0 // Vertex 4
	};
	float texCoor[vertexNumber][2] =
	{
		1.0, 0.0,
		1.0, 1.0,
		0.5, 1.0,
		0.0, 0.5,
		0.0, 0.0
	};
	for (int i = 0; i < vertexNumber; i++)
	{
		VertexBasic vertex;
		vertex.pos = { vertices[i][0], vertices[i][1], vertices[i][2] };
		vertex.normal = { 0, 0, 1 };
		vertex.texCoor = { texCoor[i][0], texCoor[i][1] };
		vertex.pos += shape.offset;
		shape.points.push_back(vertex);
	}
	shape.indices = { 0,1,4,1,2,3,1,3,4 };
	shape.vertices = shape.generateVertices();

	for(int i = 0; i < 5; i++)
	{
		icicles.push_back(shape);
	}
	icicles[0].state = LOADING;
}

void initLifeCrystal(Shape &shape)
{
	const int vertexNumber = 4;
	shape.moveSpeed = 3;
	shape.scaleFactor = 0.3;
	shape.rotateAxis = { 0,1,0 };
	shape.offset = { 0,0,0 };
	shape.position = { -2,2,-3 };
	float vertices[vertexNumber][3] =
	{
		0, 0, -0.8,//Vertex 0
		0.6, 0, 0, // vertex 1
		0, 0, 0.8,  // Vertex 2
		-0.6, 0, 0  // Vertex 3
	};
	float texCoor[vertexNumber][2] =
	{
		0.0, 5.0,
		5.0, 0.0,
		0.0, 0.0,
		5.0, 0.0
	};
	for (int i = 0; i < vertexNumber; i++)
	{
		VertexBasic vertex;
		vertex.pos = { vertices[i][0], vertices[i][1], vertices[i][2] };
		vertex.normal = { 0, 0, 1 };
		vertex.texCoor = { texCoor[i][0], texCoor[i][1] };
		vertex.pos += shape.offset;
		shape.points.push_back(vertex);
	}
	shape.indices = { 0,1,3,1,2,3 };
	shape.vertices = shape.generateVertices();
}

void initLifeCrystals(std::vector<Shape> &crystals)
{
	Shape shape;
	initLifeCrystal(shape);
	for (int i = 0; i < 3; i++)
	{
		shape.position.z += 0.5;
		crystals.push_back(shape);
	}
}

void initFlame(Shape &shape)
{
	const int vertexNumber = 8;
	shape.moveSpeed = 1;
	shape.scaleFactor = 1;
	shape.rotateAxis = { 0,1,0 };
	shape.offset = { 0,0,1 };
	float vertices[vertexNumber][3] =
	{
		0, 0, 0,//Vertex 0
		0.4, 0, 1, // vertex 1
		0.25, 0, 0.9,  // Vertex 2
		0.1, 0, 0.98, // Vertex3
		0, 0, 0.92, // Vertex 4
		-0.1, 0, 0.98,
		-0.25, 0, 0.9,
		-0.4, 0, 1
	};
	float texCoor[vertexNumber][2] =
	{
		1.0, 1.0,
		0.0, 1.0,
		0.0, 0.0,
		0.0, 1.0,
		0.0, 0.0,
		0.0, 1.0,
		0.0, 0.0,
		0.0, 1.0
	};
	for (int i = 0; i < vertexNumber; i++)
	{
		VertexBasic vertex;
		vertex.pos = { vertices[i][0], vertices[i][1], vertices[i][2] };
		vertex.normal = { 0, 0, 1 };
		vertex.texCoor = { texCoor[i][0], texCoor[i][1] };
		vertex.pos += shape.offset;
		shape.points.push_back(vertex);
	}
	shape.indices = { 0,1,2,0,2,3,0,3,4,0,4,5,0,5,6,0,6,7 };
	shape.vertices = shape.generateVertices();
}

void initFlames(std::vector<Shape> &flames)
{
	Shape shape;
	initFlame(shape);

	for (int i = 0; i < 5; i++)
	{
		flames.push_back(shape);
	}
	flames[0].state = LOADING;
}

void initIceBerg(Model &iceBerg)
{
	iceBerg.scaleFactor = 0.15;
	iceBerg.position = { 0,1,2.8 };
}

int loadIceBerg(IceBerg &iceBerg)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;
	//load
	{
		int vertexCounter = 0;
		// load initial pose
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "iceberg.obj")) {
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

				iceBerg.vertices.push_back(vertex);
			}
		}

		// load texture for enemy
		iceBerg.loadTexture("iceberg.jpg");

		/////// handle the vertices of enemy
		{
			glGenBuffers(1, &iceBerg.vbo);
			glBindBuffer(GL_ARRAY_BUFFER, iceBerg.vbo);
			glBufferData(GL_ARRAY_BUFFER, iceBerg.vertices.size() * sizeof(VertexBasic), iceBerg.vertices.data(), GL_STATIC_DRAW);

			glGenVertexArrays(1, &iceBerg.vao);
			glBindVertexArray(iceBerg.vao);

			glBindBuffer(GL_ARRAY_BUFFER, iceBerg.vbo);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), reinterpret_cast<void*>(offsetof(VertexBasic, pos)));
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, iceBerg.vbo);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), reinterpret_cast<void*>(offsetof(VertexBasic, normal)));
			glEnableVertexAttribArray(1);

			glBindBuffer(GL_ARRAY_BUFFER, iceBerg.vbo);
			glVertexAttribPointer(8, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), reinterpret_cast<void*>(offsetof(VertexBasic, texCoor)));
			glEnableVertexAttribArray(8);
		}
		return 0;
	}
}

void initEnemy(Enemy &enemy)
{
	enemy.position = { 0,0,4 };
	enemy.scaleFactor = 0.2;
	enemy.rotateAxis = { 0,1,0 };
	enemy.rotateAngle = 3.14159;
	enemy.movement.y = -0.03;
	enemy.mixFactor.increment = 0.07;
}

void initEnemies(std::vector<Enemy> &enemies)
{
	Enemy enemy;
	initEnemy(enemy);
	for (int i = 0; i < 5; i++)
	{
		enemy.position.x = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		enemy.position.z += 5.0;
		enemies.push_back(enemy);
	}
}
int loadAnivia(Anivia &anivia)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

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

				anivia.vertices.push_back(vertex);
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
				VertexBasic vertex = {};

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

				anivia.vertices[vertexCounter].pos_idle = vertex.pos;
				anivia.vertices[vertexCounter].normal_idle = vertex.normal;
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
				VertexBasic vertex = {};

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

				anivia.vertices[vertexCounter].pos_attack = vertex.pos;
				anivia.vertices[vertexCounter].normal_attack = vertex.normal;
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
				VertexBasic vertex = {};

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

				anivia.vertices[vertexCounter].pos_dead = vertex.pos;
				anivia.vertices[vertexCounter].normal_dead = vertex.normal;
				vertexCounter++;
			}
		}
	}

	// load texture for anivia
	anivia.loadTexture("anivia.png");

	/////// handle the vertices of anivia
	{
		glGenBuffers(1, &anivia.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, anivia.vbo);
		glBufferData(GL_ARRAY_BUFFER, anivia.vertices.size() * sizeof(AniviaVertex), anivia.vertices.data(), GL_STATIC_DRAW);

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
	return 0;
}
int loadEnemy(Enemy &enemy)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;
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

				enemy.vertices.push_back(vertex);
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
				VertexBasic vertex = {};

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

				enemy.vertices[vertexCounter].pos_idle = vertex.pos;
				enemy.vertices[vertexCounter].normal_idle = vertex.normal;
				vertexCounter++;
			}
		}

		//load dead pose
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "aatrox_dead.obj")) {
			std::cerr << err << std::endl;
			return EXIT_FAILURE;
		}
		// Read triangle vertices from OBJ file
		vertexCounter = 0;
		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				VertexBasic vertex = {};

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

				enemy.vertices[vertexCounter].pos_dead = vertex.pos;
				enemy.vertices[vertexCounter].normal_dead = vertex.normal;
				vertexCounter++;
			}
		}

	}

	// load texture for enemy
	enemy.loadTexture("Aatrox_Base_Mat.png");

	/////// handle the vertices of enemy
	{
		glGenBuffers(1, &enemy.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, enemy.vbo);
		glBufferData(GL_ARRAY_BUFFER, enemy.vertices.size() * sizeof(EnemyVertex), enemy.vertices.data(), GL_STATIC_DRAW);

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
		glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(EnemyVertex), reinterpret_cast<void*>(offsetof(EnemyVertex, pos_dead)));
		glEnableVertexAttribArray(6);

		glBindBuffer(GL_ARRAY_BUFFER, enemy.vbo);
		glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(EnemyVertex), reinterpret_cast<void*>(offsetof(EnemyVertex, normal_dead)));
		glEnableVertexAttribArray(7);

		glBindBuffer(GL_ARRAY_BUFFER, enemy.vbo);
		glVertexAttribPointer(8, 2, GL_FLOAT, GL_FALSE, sizeof(EnemyVertex), reinterpret_cast<void*>(offsetof(EnemyVertex, texCoor)));
		glEnableVertexAttribArray(8);
	}
	return 0;
}

void loadEnemies(std::vector<Enemy> &enemies)
{
	for (int i = 0; i < enemies.size(); i++)
	{
		loadEnemy(enemies[i]);
	}
}

int loadBoss(Boss &boss)
{

	/////// for simplified model
	{
		glGenBuffers(1, &boss.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, boss.vbo);
		glBufferData(GL_ARRAY_BUFFER, boss.vertices.size() * sizeof(BossVertex), boss.vertices.data(), GL_STATIC_DRAW);

		glGenVertexArrays(1, &boss.vao);
		glBindVertexArray(boss.vao);

		glBindBuffer(GL_ARRAY_BUFFER, boss.vbo);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BossVertex), reinterpret_cast<void*>(offsetof(BossVertex, pos)));
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, boss.vbo);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(BossVertex), reinterpret_cast<void*>(offsetof(BossVertex, normal)));
		glEnableVertexAttribArray(1);
	}


	////// LOAD MODEL WITH TEXTURE FOR ANIMATION
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;
	//load
	{
		int vertexCounter = 0;
		// load initial pose
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "boss_low.obj")) {
			std::cerr << err << std::endl;
			return EXIT_FAILURE;
		}
		// Read triangle vertices from OBJ file
		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				BossVertex vertex = {};

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

				boss.texturedVertices.push_back(vertex);
			}
		}

		//load idle pose (animation between initial and idle pose)
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "boss_high.obj")) {
			std::cerr << err << std::endl;
			return EXIT_FAILURE;
		}
		// Read triangle vertices from OBJ file
		vertexCounter = 0;
		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				VertexBasic vertex = {};

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

				boss.texturedVertices[vertexCounter].pos_idle = vertex.pos;
				boss.texturedVertices[vertexCounter].normal_idle = vertex.normal;
				vertexCounter++;
			}
		}

		//load dead pose
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "boss_attack.obj")) {
			std::cerr << err << std::endl;
			return EXIT_FAILURE;
		}
		// Read triangle vertices from OBJ file
		vertexCounter = 0;
		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				VertexBasic vertex = {};

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

				boss.texturedVertices[vertexCounter].pos_attack = vertex.pos;
				boss.texturedVertices[vertexCounter].normal_attack = vertex.normal;
				vertexCounter++;
			}
		}
	}


	// load texture for enemy
	boss.loadTexture("legenddragon-fire.png");

	/////// handle the vertices of boss
	{
		glGenBuffers(1, &boss.vbo_tex);
		glBindBuffer(GL_ARRAY_BUFFER, boss.vbo_tex);
		glBufferData(GL_ARRAY_BUFFER, boss.texturedVertices.size() * sizeof(BossVertex), boss.texturedVertices.data(), GL_STATIC_DRAW);

		glGenVertexArrays(1, &boss.vao_tex);
		glBindVertexArray(boss.vao_tex);

		glBindBuffer(GL_ARRAY_BUFFER, boss.vbo_tex);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BossVertex), reinterpret_cast<void*>(offsetof(BossVertex, pos)));
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, boss.vbo_tex);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(BossVertex), reinterpret_cast<void*>(offsetof(BossVertex, normal)));
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, boss.vbo_tex);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(BossVertex), reinterpret_cast<void*>(offsetof(BossVertex, pos_idle)));
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, boss.vbo_tex);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(BossVertex), reinterpret_cast<void*>(offsetof(BossVertex, normal_idle)));
		glEnableVertexAttribArray(3);

		glBindBuffer(GL_ARRAY_BUFFER, boss.vbo_tex);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(BossVertex), reinterpret_cast<void*>(offsetof(BossVertex, pos_attack)));
		glEnableVertexAttribArray(4);

		glBindBuffer(GL_ARRAY_BUFFER, boss.vbo_tex);
		glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(BossVertex), reinterpret_cast<void*>(offsetof(BossVertex, normal_attack)));
		glEnableVertexAttribArray(5);

		glBindBuffer(GL_ARRAY_BUFFER, boss.vbo_tex);
		glVertexAttribPointer(8, 2, GL_FLOAT, GL_FALSE, sizeof(BossVertex), reinterpret_cast<void*>(offsetof(BossVertex, texCoor)));
		glEnableVertexAttribArray(8);
	}
	return 0;
}

int loadTerrain(Terrain &terrain)
{
	////////////////terrain
	{
		glGenBuffers(1, &terrain.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, terrain.vbo);
		glBufferData(GL_ARRAY_BUFFER, terrain.vertices.size() * sizeof(terrainVertex), terrain.vertices.data(), GL_STATIC_DRAW);

		glGenVertexArrays(1, &terrain.vao);
		glBindVertexArray(terrain.vao);

		glBindBuffer(GL_ARRAY_BUFFER, terrain.vbo);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(terrainVertex), reinterpret_cast<void*>(offsetof(terrainVertex, pos)));
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, terrain.vbo);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(terrainVertex), reinterpret_cast<void*>(offsetof(terrainVertex, normal)));
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, terrain.vbo);
		glVertexAttribPointer(8, 2, GL_FLOAT, GL_FALSE, sizeof(terrainVertex), reinterpret_cast<void*>(offsetof(terrainVertex, texCoor)));
		glEnableVertexAttribArray(8);

		glBindBuffer(GL_ARRAY_BUFFER, terrain.vbo);
		glVertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, sizeof(terrainVertex), reinterpret_cast<void*>(offsetof(terrainVertex, shadow)));
		glEnableVertexAttribArray(9);
	}
	// add texture for terrain
	terrain.loadTexture("terrain.jpg");

	
	return 0;
}

void loadIcicle(Shape &icicle)
{
	icicle.loadTexture("icicle.png");
	glGenBuffers(1, &icicle.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, icicle.vbo);
	glBufferData(GL_ARRAY_BUFFER, icicle.vertices.size() * sizeof(VertexBasic), icicle.vertices.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &icicle.vao);
	glBindVertexArray(icicle.vao);

	glBindBuffer(GL_ARRAY_BUFFER, icicle.vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), reinterpret_cast<void*>(offsetof(VertexBasic, pos)));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, icicle.vbo);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), reinterpret_cast<void*>(offsetof(VertexBasic, normal)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, icicle.vbo);
	glVertexAttribPointer(8, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), reinterpret_cast<void*>(offsetof(VertexBasic, texCoor)));
	glEnableVertexAttribArray(8);
}

void loadCrystal(Shape &crystal)
{
	crystal.loadTexture("icicle.png");
	glGenBuffers(1, &crystal.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, crystal.vbo);
	glBufferData(GL_ARRAY_BUFFER, crystal.vertices.size() * sizeof(VertexBasic), crystal.vertices.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &crystal.vao);
	glBindVertexArray(crystal.vao);

	glBindBuffer(GL_ARRAY_BUFFER, crystal.vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), reinterpret_cast<void*>(offsetof(VertexBasic, pos)));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, crystal.vbo);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), reinterpret_cast<void*>(offsetof(VertexBasic, normal)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, crystal.vbo);
	glVertexAttribPointer(8, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), reinterpret_cast<void*>(offsetof(VertexBasic, texCoor)));
	glEnableVertexAttribArray(8);
}


void loadFlame(Shape &flame)
{
	flame.loadTexture("fire2.png");
	glGenBuffers(1, &flame.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, flame.vbo);
	glBufferData(GL_ARRAY_BUFFER, flame.vertices.size() * sizeof(VertexBasic), flame.vertices.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &flame.vao);
	glBindVertexArray(flame.vao);

	glBindBuffer(GL_ARRAY_BUFFER, flame.vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), reinterpret_cast<void*>(offsetof(VertexBasic, pos)));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, flame.vbo);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), reinterpret_cast<void*>(offsetof(VertexBasic, normal)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, flame.vbo);
	glVertexAttribPointer(8, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), reinterpret_cast<void*>(offsetof(VertexBasic, texCoor)));
	glEnableVertexAttribArray(8);
}

int main() {
	//init
	initAnivia(anivia);
	initIcicles(icicles);
	initFlames(flames);
	initLifeCrystals(lifeCrystals);
	//initEnemy(enemy);
	initBoss(boss);
	initEnemies(enemies);
	initIceBerg(iceBerg);

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

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Frozen Flame", nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create OpenGL context!" << std::endl;
		std::cout <<  "Press enter to close."; getchar();
		return EXIT_FAILURE;
	}

	glfwSetKeyCallback(window, keyboardHandler);
	//glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);
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

	}
	////////////////////////// Load vertices of model
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	//defination of position (offset)
	glm::vec3 aniviaPosition = { 0.0, 0.0, 0.0 };

	//defination of vertices
	std::vector<VertexBasic> vertices;
	//std::vector<AniviaVertex> aniviaVertices;
	//std::vector<EnemyVertex> enemyVertices;
	std::vector<AniviaVertex> aniviaHeadVertices;
	
	loadAnivia(anivia);
	//loadEnemy(enemy);
	loadEnemies(enemies);
	loadTerrain(terrain);
	for (int i = 0; i < icicles.size(); i++)
	{
		loadIcicle(icicles[i]);
	}

	for (int i = 0; i < flames.size(); i++)
	{
		loadFlame(flames[i]);
	}

	for (int i = 0; i < lifeCrystals.size(); i++)
	{
		loadCrystal(lifeCrystals[i]);
	}


	loadBoss(boss);
	loadIceBerg(iceBerg);

	//////////////////// Create Vertex Buffer Object
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexBasic), vertices.data(), GL_STATIC_DRAW);

	// Bind vertex data to shader inputs using their index (location)
	// These bindings are stored in the Vertex Array Object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// The position vectors should be retrieved from the specified Vertex Buffer Object with given offset and stride
	// Stride is the distance in bytes between vertices
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), reinterpret_cast<void*>(offsetof(VertexBasic, pos)));
	glEnableVertexAttribArray(0);

	// The normals should be retrieved from the same Vertex Buffer Object (glBindBuffer is optional)
	// The offset is different and the data should go to input 1 instead of 0
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), reinterpret_cast<void*>(offsetof(VertexBasic, normal)));
	glEnableVertexAttribArray(1);


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
	mainCamera.position = glm::vec3(0.0f, 12.0f, 0.0f);
	//mainCamera.position = glm::vec3(0.0f, 9.5f, 2.5f);
	//mainCamera.forward  = -mainCamera.position;
	mainCamera.forward = glm::vec3(0.0f, -1.0f, -0.0f);

	Camera lightSource;
	lightSource.aspect = WIDTH / (float)HEIGHT;
	lightSource.position = glm::vec3(-3.0f, 10.0f, 0.1f);
	lightSource.forward = -lightSource.position;


	StateType lastState = IDLE;
	double lastShot = glfwGetTime();

	// Main loop
	while (!glfwWindowShouldClose(window)) {
		double timeInterval = glfwGetTime() - lastFrameTime;
		
		if (timeInterval < 1.0 / maxFrameRate)
			continue;
		lastFrameTime = glfwGetTime();
		
		//update 
		anivia.move(mainCamera);
		anivia.updateMixFactor(timeInterval);
		
		for (int i = 0; i < enemies.size(); i++)
		{
			Enemy &enemy = enemies[i];
			enemy.move(mainCamera);
			enemy.updateMixFactor(timeInterval);
			bool contacted = false;
			contacted = enemy.detectCollision(anivia);
			if (contacted)
			{
				if (lifeCrystals.size() == 0)
					anivia.state = DEAD;
				else
					lifeCrystals.pop_back();
			}
		}
		
		boss.updateMixFactor(timeInterval);
		if (boss.state == DEAD)
		{
			boss.mixFactor.dead = 0.0;
		}
		boss.update(); // update the vertices according to state

		terrain.update();

		//zoom effect after boss death
		if (boss.state == DEAD) {
			if(mainCamera.position.y>=9.5){
				glm::vec3 zoom = { 0, 0.01, 0.01 };
				mainCamera.updatePosition(zoom);
			}
		}
		
		for(int i = 0; i < icicles.size(); i++)
		{
			Shape &icicle = icicles[i];

			glm::vec2 screenCoor = icicle.getScreenCoor(mainCamera);
			icicle.update(mainCamera, anivia.position, mouse.screenCoor, timeInterval);
			if (icicle.state == SHOT)
			{				
				icicle.detectCollision(boss);
				for (int j = 0; j < enemies.size(); j++)
				{
					Enemy &enemy = enemies[j];
					icicle.detectCollision(enemy);
				}
			}
		}
				
		{
			double currentTime = glfwGetTime();
			if (currentTime - lastShot > boss.coolDownTime && boss.state != DEAD)
			{
				flames[currentFlame].state = TRIGGERED;
				currentFlame = (currentFlame + 1) % flames.size();
				flames[currentFlame].state = LOADING;
				lastShot = currentTime;
			}
			else if (boss.state == DEAD)
			{
				flames[currentFlame].state = WAITING;
			}
		}

		for (int i = 0; i < flames.size(); i++)
		{
			Shape &flame = flames[i];

			glm::vec2 screenCoor = flame.getScreenCoor(mainCamera);
			flame.update(mainCamera, boss.position, anivia.getScreenCoor(mainCamera), timeInterval, 0.8);
			if (flame.state == TRIGGERED)
			{
				flame.state = SHOT;
				boss.mixFactor.attack = 1.0;
			}
			else if (flame.state == SHOT)
			{
				bool damaged = flame.detectCollision(anivia);
				if (damaged)
				{
					if (lifeCrystals.size() == 0)
						anivia.state = DEAD;
					else
						lifeCrystals.pop_back();
				}
					
			}

		}
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
			//glDrawArrays(GL_TRIANGLES, 0, vertices.size());


			// .... HERE YOU MUST ADD THE CORRECT UNIFORMS FOR RENDERING THE SHADOW MAP
			glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(lightSource.voMatrix()));
			// Bind vertex data


			glBindVertexArray(anivia.vao);
			anivia.passUniform(shadowProgram);
			glDrawArrays(GL_TRIANGLES, 0, anivia.vertices.size());


			for (int i = 0; i < enemies.size(); i++)
			{
				Enemy &enemy = enemies[i];
				glBindVertexArray(enemy.vao);
				enemy.passUniform(shadowProgram);
				glDrawArrays(GL_TRIANGLES, 0, enemy.vertices.size());
			}

			for (int j = 0; j < icicles.size(); j++)
			{
				Shape & icicle = icicles[j];
				glBindVertexArray(icicle.vao);
				icicle.passUniform(shadowProgram);
				glDrawArrays(GL_TRIANGLES, 0, icicle.vertices.size());
			}


			float scaleFactor = boss.scaleFactor;
			boss.scaleFactor = 0.22;

			boss.position.z -= 0.1;
			boss.position.y -= 0.5;
			glBindVertexArray(boss.vao_tex);
			boss.passUniform(shadowProgram, false, false, false, true);
			glDrawArrays(GL_TRIANGLES, 0, boss.texturedVertices.size());

			boss.position.z += 0.1;
			boss.position.y += 0.5;
			boss.scaleFactor = scaleFactor;


			for (int j = 0; j < flames.size(); j++)
			{
				Shape & flame = flames[j];
				{
					glBindBuffer(GL_ARRAY_BUFFER, flame.vbo);
					glBufferSubData(GL_ARRAY_BUFFER, 0, flame.vertices.size() * sizeof(VertexBasic), flame.vertices.data());
					//glBindVertexArray(terrain.vao);
				}
				glBindVertexArray(flame.vao);
				flame.passUniform(shadowProgram);
				glDrawArrays(GL_TRIANGLES, 0, flame.vertices.size());
				//std::cerr << flame.state;
			}


			// Unbind the off-screen framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);



		}

		// Bind the shader
		glUseProgram(mainProgram); 
		glm::mat4 mvp;
		if (lightView == false)
		{
			updateCamera(mainCamera);
			mvp = mainCamera.vpMatrix();
		}
		else
		{

			updateCamera(lightSource);

			mvp = lightSource.voMatrix();
		}

		glUniformMatrix4fv(glGetUniformLocation(mainProgram, "mvp"), 1, GL_FALSE, glm::value_ptr(mvp));
		glUniform3fv(glGetUniformLocation(mainProgram, "viewPos"), 1, glm::value_ptr(mainCamera.position));
		glUniform1f(glGetUniformLocation(mainProgram, "time"), static_cast<float>(glfwGetTime()));
		glUniformMatrix4fv(glGetUniformLocation(mainProgram, "lightMVP"), 1, GL_FALSE, glm::value_ptr(lightSource.voMatrix()));
		glUniform3fv(glGetUniformLocation(mainProgram, "lightPos"), 1, glm::value_ptr(lightSource.position));
		
		

		// Bind vertex data

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

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glBindVertexArray(anivia.vao);
		anivia.passUniform(mainProgram);
		glDrawArrays(GL_TRIANGLES, 0, anivia.vertices.size());


		for (int i = 0; i < enemies.size(); i++)
		{
			Enemy &enemy = enemies[i];
			glBindVertexArray(enemy.vao);
			enemy.passUniform(mainProgram);
			glDrawArrays(GL_TRIANGLES, 0, enemy.vertices.size());
		}
		

		//// update boss vertices
		{
			glBindBuffer(GL_ARRAY_BUFFER, boss.vbo);
			glBufferSubData(GL_ARRAY_BUFFER, 0, boss.vertices.size() * sizeof(BossVertex), boss.vertices.data());
		}
		/*glBindVertexArray(boss.vao);
		boss.passUniform(mainProgram, true, true, false);
		glDrawArrays(GL_TRIANGLES, 0, boss.vertices.size());
		
		*/

		if (boss.state == IDLE) {
			bossHit = false;
		}
		else {
			bossHit = true;
		}

		glBindVertexArray(boss.vao);
		if (boss.state != IDLE) {
			boss.passUniform(mainProgram, true, true, false);

			glDrawArrays(GL_TRIANGLES, 0, boss.vertices.size());
		}
		//boss.passUniform(mainProgram, true, true, false);

		float scaleFactor = boss.scaleFactor;
		boss.scaleFactor = 0.22;
		boss.position.z -= 0.1;
		boss.position.y -= 0.5;

		glBindVertexArray(boss.vao_tex);
		boss.passUniform(mainProgram, false, false, bossHit, true);
		glDrawArrays(GL_TRIANGLES, 0, boss.texturedVertices.size());

		

		/*glBindVertexArray(boss.vao_tex);
		boss.passUniform(mainProgram, false, false, false);
		glDrawArrays(GL_TRIANGLES, 0, boss.texturedVertices.size());*/
		
		boss.scaleFactor = scaleFactor;
		boss.position.z += 0.1;
		boss.position.y += 0.5;
		// update terrain vertices
		{
			glBindBuffer(GL_ARRAY_BUFFER, terrain.vbo);
			glBufferSubData(GL_ARRAY_BUFFER, 0, terrain.vertices.size() * sizeof(terrainVertex), terrain.vertices.data());
			//glBindVertexArray(terrain.vao);
		}

		glBindVertexArray(terrain.vao);
		terrain.passUniform(mainProgram);
		glDrawArrays(GL_TRIANGLES, 0, terrain.vertices.size());
		
		// update icicle vertices
		for (int j = 0; j < icicles.size(); j++)
		{
			Shape & icicle = icicles[j];
			for (int i = 0; i < icicle.vertices.size(); i++)
			{
				icicle.vertices[i].texCoor.x -= 0.01;
				icicle.vertices[i].texCoor.y += 0.01;
			}
			{
				glBindBuffer(GL_ARRAY_BUFFER, icicle.vbo);
				glBufferSubData(GL_ARRAY_BUFFER, 0, icicle.vertices.size() * sizeof(VertexBasic), icicle.vertices.data());
				//glBindVertexArray(terrain.vao);
			}
			glBindVertexArray(icicle.vao);
			icicle.passUniform(mainProgram);
			glDrawArrays(GL_TRIANGLES, 0, icicle.vertices.size());
		}
		
		// update flame vertices
		for (int j = 0; j < flames.size(); j++)
		{
			Shape & flame = flames[j];
			for (int i = 0; i < flame.vertices.size(); i++)
			{
				flame.vertices[i].texCoor.x += 0.01;
				flame.vertices[i].texCoor.y -= 0.01;
			}
			{
				glBindBuffer(GL_ARRAY_BUFFER, flame.vbo);
				glBufferSubData(GL_ARRAY_BUFFER, 0, flame.vertices.size() * sizeof(VertexBasic), flame.vertices.data());
				//glBindVertexArray(terrain.vao);
			}
			glBindVertexArray(flame.vao);
			flame.passUniform(mainProgram);
			glDrawArrays(GL_TRIANGLES, 0, flame.vertices.size());
			//std::cerr << flame.state;
		}

		for (int j = 0; j < lifeCrystals.size(); j++)
		{
			Shape & crystal = lifeCrystals[j];
			//for (int i = 0; i < flame.vertices.size(); i++)
			//{
			//	flame.vertices[i].texCoor.x += 0.01;
			//	flame.vertices[i].texCoor.y -= 0.01;
			//}
			{
				glBindBuffer(GL_ARRAY_BUFFER, crystal.vbo);
				glBufferSubData(GL_ARRAY_BUFFER, 0, crystal.vertices.size() * sizeof(VertexBasic), crystal.vertices.data());
				//glBindVertexArray(terrain.vao);
			}
			glBindVertexArray(crystal.vao);
			crystal.passUniform(mainProgram);
			glDrawArrays(GL_TRIANGLES, 0, crystal.vertices.size());
			//std::cerr << flame.state;
		}


		glBindVertexArray(iceBerg.vao);
		float opacity;
		switch (boss.state)
		{
		case IDLE:
			opacity = 0.0;
			break;
		case DAMAGE1:
			opacity = 0.1;
			break;
		case DAMAGE2:
			opacity = 0.3;
			break;
		case DAMAGE3:
			opacity = 0.5;
			break;
		case DEAD:
			opacity = 0.8;
			break;
		default:
			break;
		}
		iceBerg.passUniform(mainProgram, opacity);
		glDrawArrays(GL_TRIANGLES, 0, iceBerg.vertices.size());

		// Present result to the screen
		glfwSwapBuffers(window);

	}

	glDeleteFramebuffers(1, &framebuffer);

	glDeleteTextures(1, &texShadow);

	glfwDestroyWindow(window);
	
	glfwTerminate();

    return 0;
}