#include <vector>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/vector_angle.hpp>
//

enum StateType
{
	IDLE = 0,
	ATTACK = 1,
	DEAD = 2,
	TRIGGERED = 3,
	SHOT = 4,
	STATECOUNT
};

struct MixFactor
{
	float idle = 0.0;
	float attack = 0.0;
	float dead = 0.0;
	float increment = 0.001;
};

struct VertexBasic {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texCoor;
};
struct terrainVertex :VertexBasic
{
	glm::vec3 shadow;
};
struct EnemyVertex :VertexBasic {
	glm::vec3 pos_idle;
	glm::vec3 normal_idle;
	glm::vec3 pos_dead;
	glm::vec3 normal_dead;
};
struct AniviaVertex :VertexBasic
{
	glm::vec3 pos_idle;
	glm::vec3 normal_idle;
	glm::vec3 pos_attack;
	glm::vec3 normal_attack;
	glm::vec3 pos_dead;
	glm::vec3 normal_dead;
};



class Model
{	
public:
	static int textureCount;
	glm::vec3 position = { 0,0,0 };
	glm::vec3 rotateAxis = { 0,1,0 };
	glm::vec2 screenCoor = { 0,0 };
	float rotateAngle = 0.0;
	float scaleFactor = 1.0;
	GLuint texture;
	int textureNumber;
	GLuint vao, vbo;
	void loadTexture(char* fileName)
	{
		int width, height, channels;
		stbi_uc* pixels = stbi_load(fileName, &width, &height, &channels, 3);

		// Create Texture

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		// Upload pixels into texture
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

		//// Set behaviour for when texture coordinates are outside the [0, 1] range
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		//// Set interpolation for texture sampling (GL_NEAREST for no interpolation)
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Set behaviour for when texture coordinates are outside the [0, 1] range
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Set interpolation for texture sampling (GL_NEAREST for no interpolation)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		textureNumber = textureCount++;
	}
	void passUniform(GLuint program)
	{
		glUniform3fv(glGetUniformLocation(program, "pos_offset"), 1, glm::value_ptr(position));
		glUniform3fv(glGetUniformLocation(program, "rotateAxis"), 1, glm::value_ptr(rotateAxis));
		glUniform1f(glGetUniformLocation(program, "rotateAngle"), rotateAngle);
		glUniform1f(glGetUniformLocation(program, "scaleFactor"), scaleFactor);
		glUniform1f(glGetUniformLocation(program, "mixFactor_idle"), 0.0);
		glUniform1f(glGetUniformLocation(program, "mixFactor_attack"), 0.0);
		glUniform1f(glGetUniformLocation(program, "mixFactor_dead"), 0.0);
		glActiveTexture(GL_TEXTURE0 + textureNumber);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(glGetUniformLocation(program, "tex"), textureNumber);
		glUniform1f(glGetUniformLocation(program, "useShadow"), false);

	}
	glm::vec2 getScreenCoor(Camera camera)
	{
		glm::vec4 homoScreenCoor = camera.vpMatrix()*glm::vec4(position, 1.0);
		screenCoor = { homoScreenCoor.x / homoScreenCoor.w, homoScreenCoor.y / homoScreenCoor.w };
		screenCoor = screenCoor * 0.5f + glm::vec2(0.5, 0.5);
		return screenCoor;
	}
};

class Character: public Model
{
public:
	StateType state = IDLE;
	MixFactor mixFactor;
	float moveSpeed = 0.005;
	glm::vec3 movement = { 0,0,0 };
	float safeDistance = 1.0;
	
	void move(Camera camera)
	{
		glm::vec3 forward = glm::normalize(camera.forward);
		glm::vec3 up = glm::normalize(camera.up);
		glm::vec3 right = glm::normalize(glm::cross(forward, up));
		glm::vec3 realUp = glm::normalize(glm::cross(right, forward));
		position += right * movement.x + realUp * movement.y + forward * movement.z;
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

	void passUniform(GLuint program)
	{
		Model::passUniform(program);
		glUniform1f(glGetUniformLocation(program, "mixFactor_idle"), mixFactor.idle);
		glUniform1f(glGetUniformLocation(program, "mixFactor_attack"), mixFactor.attack);
		glUniform1f(glGetUniformLocation(program, "mixFactor_dead"), mixFactor.dead);
	}
};

struct Anivia : public Character
{
public:
	std::vector<AniviaVertex> vertices;
};

class Enemy : public Character
{
public:
	std::vector<EnemyVertex> vertices;
};

class Terrain: public Model
{
public:
	int NbVertX, NbVertY;
	int startingRow = 0; //draw from this row
	std::vector <std::vector<terrainVertex>> grid;
	float updateInterval = 1.0;
	std::vector <terrainVertex> vertices;
	double lastUpdateTime = 0;
	double lastFrameTime = 0;
	Terrain(int NbVertX, int NbVertY, glm::vec3 lightDir)
	{
		position = { -5.0,-5.0,-5.0 };
		rotateAxis = { 1.0,0.0,0.0 };
		rotateAngle = 0;
		this->NbVertX = NbVertX;
		this->NbVertY = NbVertY;
		lastUpdateTime = glfwGetTime();
		lastFrameTime = lastUpdateTime;
		generateTerrain(lightDir);
	}
	void generateTerrain(glm::vec3 lightDir)
	{
		// i - row; j - column
		for (int i = 0; i < NbVertY; i++)
		{
			std::vector<terrainVertex> row;
			for (int j = 0; j < NbVertX; j++)
			{
				terrainVertex vertex;
				float height;
				height = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				//height = abs(sin(i*j));
				vertex.pos = { j, height, i};
				vertex.texCoor = { j / float(NbVertX), i / float(NbVertY) };
				vertex.normal = { 0,0,0 };
				vertex.shadow = { 1,1,1 };
				row.push_back(vertex);
			}
			grid.push_back(row);
		}
		calculateNormals();

		computeShadow(lightDir);
		generateTriangles();
	}
	void passUniform(GLuint program)
	{
		Model::passUniform(program);

		glUniform1f(glGetUniformLocation(program, "useShadow"), true);
	}

	void calculateNormals()
	{
		for (int i = 0; i < NbVertY; i++)
		{
			for (int j = 0; j < NbVertX - 1; j++)
			{
				VertexBasic *vertex_1, *vertex_2, *vertex_3, *vertex_4;
				vertex_1 = &grid[i][j];
				vertex_2 = &grid[i][j + 1];

				if (i == NbVertY - 1)
				{
					vertex_3 = &grid[0][j + 1];
					vertex_4 = &grid[0][j];
				}
				else
				{
					vertex_3 = &grid[(i + 1)][j + 1];
					vertex_4 = &grid[(i + 1)][j];
				}

				glm::vec3 normal_1, normal_2;
				normal_1 = glm::triangleNormal(vertex_1->pos, vertex_2->pos, vertex_3->pos);
				normal_2 = glm::triangleNormal(vertex_3->pos, vertex_4->pos, vertex_1->pos);

				vertex_1->normal += normal_1;
				vertex_2->normal += normal_1;
				vertex_3->normal += normal_1;
				
				vertex_3->normal = normal_2;
				vertex_4->normal = normal_2;
				vertex_1->normal = normal_2;
			}
		}
	}

	void generateTriangles()
	{
		for (int i = 0; i < NbVertY; i++)
		{
			for (int j = 0; j < NbVertX - 1; j++)
			{
				terrainVertex vertex_1, vertex_2, vertex_3, vertex_4;
				vertex_1 = grid[i][j];
				vertex_2 = grid[i][j + 1];

				if (i == NbVertY - 1)
				{
					vertex_3 = grid[0][j + 1];
					vertex_4 = grid[0][j];
					vertex_3.pos.z += NbVertY;
					vertex_4.pos.z += NbVertY;
					vertex_3.texCoor.y = 1.0;
					vertex_4.texCoor.y = 1.0;
				}
				else
				{
					vertex_3 = grid[(i + 1)][j + 1];
					vertex_4 = grid[(i + 1)][j];
				}

				vertex_1.normal = glm::normalize(vertex_1.normal);
				vertex_2.normal = glm::normalize(vertex_2.normal);
				vertex_3.normal = glm::normalize(vertex_3.normal);
				vertex_4.normal = glm::normalize(vertex_4.normal);

				vertices.push_back(vertex_1);
				vertices.push_back(vertex_2);
				vertices.push_back(vertex_3);

				vertices.push_back(vertex_3);
				vertices.push_back(vertex_4);
				vertices.push_back(vertex_1);
			}
		}
	}

	void computeShadow(glm::vec3 lightDir)
	{
		// i - row; j - column
		for (int i = 0; i < NbVertY; i++)
		{
			for (int j = 0; j < NbVertX; j++)
			{
				bool inShadow = false;
				terrainVertex vertex = grid[i][j];
				for (int ii = 0; ii < NbVertY; ii++)
				{
					for (int jj = 0; jj < NbVertX - 1; jj++)
					{
						terrainVertex vertex_1, vertex_2, vertex_3, vertex_4;
						vertex_1 = grid[(i + ii) % NbVertY][jj];
						vertex_2 = grid[(i + ii) % NbVertY][jj+ 1];
						vertex_3 = grid[(i + ii + 1) % NbVertY][jj + 1];
						vertex_4 = grid[(i + ii + 1) % NbVertY][jj];
						
						if (ii == 0 && jj == j || ii == 0 && jj + 1 == j || ii + 1 == 0 && jj == j || ii + 1 == 0 && jj + 1 == j)
							continue;

						if (i + ii >= NbVertY)
						{
							vertex_1.pos.z += NbVertY;
							vertex_2.pos.z += NbVertY;
							vertex_3.pos.z += NbVertY;
							vertex_4.pos.z += NbVertY;
						}

						glm::vec3 baryPosition;
						inShadow = glm::intersectRayTriangle(vertex.pos, -lightDir, vertex_1.pos, vertex_2.pos, vertex_3.pos, baryPosition);
						if (inShadow == true)
							break;
						inShadow = glm::intersectRayTriangle(vertex.pos, -lightDir, vertex_3.pos, vertex_4.pos, vertex_1.pos, baryPosition);
						if (inShadow == true)
							break;
					}
					if (inShadow == true)
						break;
				}
				if (inShadow == true)
				{
					grid[i][j].shadow = { 0,0,0 };
					std::cerr << i << '\t' << j << std::endl;
				}
					
			}
		}
	}


	void update()
	{
		double currentTime = glfwGetTime();
		position.z -= (currentTime - lastFrameTime) / updateInterval;
		lastFrameTime = currentTime;

		if (currentTime - lastUpdateTime < updateInterval)
			return;
		lastUpdateTime = currentTime;
		int startingIndex = 2 * 3 * (NbVertX - 1) * startingRow;
		for (int i = 0; i < 2 * 3 * (NbVertX - 1); i++)
		{
			vertices[startingIndex + i].pos.z += NbVertY;
		}
		startingRow++;
		startingRow %= NbVertY;
	}
};


class Shape : public Model
{
public:
	std::vector<VertexBasic> points;
	std::vector<int> indices;
	glm::vec3 offset = { 0,0,0 };
	StateType state = IDLE;
	float moveSpeed = 0;
	glm::vec3 moveNormal = { 0,0,0 };
	std::vector<VertexBasic> vertices;
	void fire(Camera camera, glm::vec2 mouseScreenCoor)
	{
		state == SHOT;
		glm::vec3 forward = glm::normalize(camera.forward);
		glm::vec3 up = glm::normalize(camera.up);
		glm::vec3 right = glm::normalize(glm::cross(forward, up));
		glm::vec3 realUp = glm::normalize(glm::cross(right, forward));
		
		getScreenCoor(camera);

		double angle;
		angle = glm::orientedAngle(glm::vec2(0.0, 1.0), glm::normalize(mouseScreenCoor - screenCoor));
		angle = -angle;

		moveNormal = right * float(sin(angle)) + realUp * float(cos(angle));
		
	}
	void move()
	{
		position += moveNormal * moveSpeed;
	}

	void update(Camera camera, glm::vec3 followPosition, glm::vec2 mouseScreenCoor)
	{
		glm::vec2 screenCoor = getScreenCoor(camera);
		double angle = glm::orientedAngle(glm::vec2(0.0, 1.0), glm::normalize(mouseScreenCoor - screenCoor));

		if (state == IDLE)
		{
			rotateAxis = { 0, 1, 0 };
			rotateAngle = -angle;
			position = followPosition;
		}
		else if (state == TRIGGERED)
		{
			fire(camera, mouseScreenCoor);
		}
		else if (state == SHOT)
		{
			move();
		}
	}

	std::vector<VertexBasic> generateVertices()
	{
		std::vector<VertexBasic> vertices;
		for (int i = 0; i < indices.size(); i++)
		{
			vertices.push_back(points[indices[i]]);
		}
		return vertices;
	}

	void detectCollision(Character &character)
	{
		float distance = 0.0;
		distance = glm::distance(position, character.position);
		if (distance <= character.safeDistance)
		{
			character.state = DEAD;
			character.movement = { 0, 0, 0.005 };
		}
	}
};