#include <glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include "shader_code.h"
#include "shader.h"
#include "plane.h"
#include "vertex.h"
#include "texture.h"
#include "gameobject.h"
#include "engine.h"
#include <sndfile.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include <iostream>

double lastX = 400, lastY = 300; // initial mouse position
float yaw = 0.8f;   // rotation around Y-axis (horizontal)
float pitch = 0.5f; // rotation around X-axis (vertical)
bool firstMouse = true;

float fov = 0.0f;

glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 horizontalCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraPos = glm::vec3(0.0f, 2.5f, 7.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float dx = xpos - lastX;
	float dy = ypos - lastY;
	lastX = xpos;
	lastY = ypos;

	constexpr float sensitivity = 0.2f;
	dx *= sensitivity;
	dy *= sensitivity;

	yaw += dx;
	pitch += -dy;

	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;

	glm::vec3 dir;
	dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	dir.y = sin(glm::radians(pitch));
	dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(dir);
	horizontalCameraFront = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	fov -= (float)yoffset;      // scroll up = zoom in, scroll down = zoom out
	if (fov < 1.0f) fov = 1.0f; // clamp minimum FOV
	if (fov > 90.0f) fov = 90.0f; // clamp maximum FOV
}

int main() {
	Engine engine("OpenGL");

	Shader shader(phongVertex, phongFragment);

	Mesh* planeModel = new Mesh(planeVertices, planeIndices);
	GameObject plane(planeModel, "resources/models/wood.png");
	plane.SetTransform(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(100.0f));
	plane.drawShadow = false;
	delete planeModel;

	Model* chairModel = new Model("resources/models/chair.obj");
	GameObject chair(chairModel, "resources/models/chair.png");
	chair.SetTransform(glm::vec3(1.0f,0.01f,0.0f), glm::vec3(0.f), glm::vec3(3.0f));

	Model* tableModel = new Model("resources/models/table.obj");
	GameObject table(tableModel, "resources/models/table.png");
	table.SetTransform(glm::vec3(1.0f,0.01f,2.5f), glm::vec3(0.f), glm::vec3(5.0f));
	delete tableModel;

	Model* plantModel = new Model("resources/models/plant.obj");
	GameObject plant(plantModel, "resources/models/plant.png");
	plant.SetTransform(glm::vec3(3.0f, -0.0f, -3.f), glm::vec3(0.f), glm::vec3(2.f));
	delete plantModel;

	Model* appleModel = new Model("resources/models/apple.obj");
	GameObject apple(appleModel, "resources/models/apple.png");
	apple.SetTransform(glm::vec3(1.0f, 1.75f, 2.5f), glm::vec3(0.f), glm::vec3(5.f));
	delete appleModel;

	engine.registerGameObject(plane);
	engine.registerGameObject(chair);
	engine.registerGameObject(table);
	engine.registerGameObject(plant);
	engine.registerGameObject(apple);
	
	AudioSourceProperties appleSource = {};
	appleSource.audioFile = "resources/crabrave.wav";
	appleSource.relative = false;
	appleSource.looping = true;
	appleSource.gain = 1.0f;
	SpatialProperties appleSourceSpatialProps = {};
	appleSourceSpatialProps.position = glm::vec3(1.0f, 1.75f, 2.5f);
	appleSourceSpatialProps.referenceDistance = 2.f;
	appleSourceSpatialProps.rolloffFactor = 0.5;
	appleSource.spatialProps = appleSourceSpatialProps;
	engine.registerAudioSource(appleSource);

	AudioSourceProperties ambientSource = {};
	ambientSource.audioFile = "resources/ambient.wav";
	ambientSource.relative = true;
	ambientSource.looping = true;
	ambientSource.gain = 0.7f;
	engine.registerAudioSource(ambientSource);

	fov = engine.fov_deg;
	engine.mouse_callback = mouse_callback;
	engine.scroll_callback = scroll_callback;

	constexpr static float radius = 10.0f;
	const static glm::vec3 target = glm::vec3(0.0f);

	GLFWwindow* window = engine.getWindow();
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	engine.manualView = true;

	engine.startPlayback();

	Light sceneLight = { glm::vec3(0.0f,5.0f,0.0f),glm::vec3(1.0f) };

	float lightCX = 0.0f;
	float lightCY = 5.0f;
	float lightCZ = 0.0f;
	float lightR = 3.f;

	float theta = 0.0f;
	constexpr float angularSpeed = 0.5f;

	while (engine.isRunning()) {
		engine.clear();

		theta += angularSpeed * engine.deltaTime;

		// orbit the light
		float lightX = lightCX + lightR * cos(theta); 
		float lightZ = lightCZ + lightR * sin(theta);
		sceneLight.pos = glm::vec3(lightX, lightCY, lightZ); // fixed height

		const float cameraSpeed = (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ? 8.5f : 4.5f) * engine.deltaTime;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			cameraPos += cameraSpeed * horizontalCameraFront;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			cameraPos -= cameraSpeed * horizontalCameraFront;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			cameraPos -= glm::normalize(glm::cross(horizontalCameraFront, cameraUp)) * cameraSpeed;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			cameraPos += glm::normalize(glm::cross(horizontalCameraFront, cameraUp)) * cameraSpeed;
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, 1);

		engine.setViewMatrix(glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp));
		engine.fov_deg = fov;

		engine.setListener(cameraPos, cameraFront, cameraUp);

		shader.use();
		shader.setVec3("viewPos", cameraPos);

		glDepthFunc(GL_LEQUAL);
		glDisable(GL_CULL_FACE);
		engine.renderSkybox();
		glDepthFunc(GL_LESS);

		glDisable(GL_CULL_FACE);
		engine.render(shader, sceneLight);

		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	engine.terminate();
}