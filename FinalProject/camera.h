// Library for vertex and matrix math
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Library for window creation and event handling
#include <GLFW/glfw3.h>

struct Camera
{
	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 up;
	
	float near;
	float far;

	float     fov;
	float     aspect;

	Camera::Camera()
	: position(glm::vec3(0, 1, 0))
	, forward (glm::vec3(0, 0, 0))
	, up      (glm::vec3(0, 0, 1)) 
	, fov     (glm::pi<float>() / 4.f)
	, aspect  (1.f)
	, near    (0.1f)
	, far     (30.f)
	{}

	glm::mat4 vMatrix() const
	{
		return glm::lookAt(position, position+forward, up);
	}

	glm::mat4 pMatrix() const
	{
		return glm::perspective(fov, aspect, near, far);
	}

	glm::mat4 vpMatrix() const
	{
		return pMatrix() * vMatrix();
	}

	glm::mat4 voMatrix() const
	{
		glm::mat4 oMatrix;
		oMatrix = glm::ortho<float>(-8.0, 8.0, -8, 8,0, 30);
		return oMatrix * vMatrix();
	}


	void updatePosition(glm::vec3 speed)
	{
		forward = glm::normalize(forward);
		up      = glm::normalize(up);
		glm::vec3 right = glm::normalize(glm::cross(forward, up));

		position += speed.z * forward + speed.y * up + speed.x * right;
	}
	
	void updateDirection(glm::vec2 rotSpeed)
	{
		if (rotSpeed.x == 0.f && rotSpeed.y == 0.f) return;

		const float sp = 0.005f;

		forward = glm::normalize(forward);
		glm::vec3 right = glm::normalize(glm::cross(forward, glm::normalize(up)));
		glm::vec3 realUp = glm::normalize(glm::cross(right, forward));

		forward = glm::normalize(forward + sp * rotSpeed.x * right - sp * rotSpeed.y * realUp);
	}
};


glm::vec3 camSpeed    = glm::vec3(0, 0, 0);
glm::vec2 camRotSpeed = glm::vec2(0, 0);

bool      rightMousePressed = false;
glm::vec2 cursorPos = glm::vec2(0.0, 0.0);


// Key handle function
void cameraKeyboardHandler(int key, int action)
{
	const float sp = 0.05f;

	switch (key) 
	{
	case GLFW_KEY_LEFT:
		if (action == GLFW_PRESS)   camSpeed.x =  -sp;
		if (action == GLFW_RELEASE) camSpeed.x =  0.0;
		break;
	case GLFW_KEY_RIGHT:
		if (action == GLFW_PRESS)   camSpeed.x =  sp;
		if (action == GLFW_RELEASE) camSpeed.x =  0.0;
		break;
	case GLFW_KEY_UP:
		if (action == GLFW_PRESS)   camSpeed.z =  sp;
		if (action == GLFW_RELEASE) camSpeed.z =  0.0;
		break;
	case GLFW_KEY_DOWN:
		if (action == GLFW_PRESS)   camSpeed.z = -sp;
		if (action == GLFW_RELEASE) camSpeed.z =  0.0;
		break;
	case GLFW_KEY_R:
		if (action == GLFW_PRESS)   camSpeed.y =  sp;
		if (action == GLFW_RELEASE) camSpeed.y =  0.0;
		break;
	case GLFW_KEY_F:
		if (action == GLFW_PRESS)   camSpeed.y = -sp;
		if (action == GLFW_RELEASE) camSpeed.y =  0.0;
		break;
	default:
		break;
	}
}


// Mouse button handle function
void camMouseButtonHandler(int button, int action)
{
	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS)   rightMousePressed = true;
	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE) rightMousePressed = false;
}

void camCursorPosHandler(double xpos, double ypos)
{

	if (!rightMousePressed) {
		camRotSpeed = glm::vec2(0, 0);
	} else {
		camRotSpeed = glm::vec2(xpos, ypos) - cursorPos;
	}

	cursorPos.x = (float)xpos;
	cursorPos.y = (float)ypos;
}

void updateCamera(Camera& camera) 
{
	camera.updatePosition(camSpeed);
	camera.updateDirection(camRotSpeed);
	camRotSpeed = glm::vec2(0, 0);
}

