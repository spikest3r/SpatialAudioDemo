#include <glad/glad.h>
#include "stb_image.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <functional>

#include "shader.h"
#include "vertex.h"
#include "texture.h"
#include "gameobject.h"
#include "engine.h"

#include <stdio.h>
#include <vector>
#include <sndfile.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>


float skyboxVertices[] = {
	// positions
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};

const char* skyVertex = R"(#version 330 core
layout (location = 0) in vec3 position;
out vec3 TexCoords;
uniform mat4 projection;
uniform mat4 view;
void main()
{
gl_Position = projection * view * vec4(position, 1.0);
TexCoords = position;
}
)";

const char* skyFragment = R"(#version 330 core
in vec3 TexCoords;
out vec4 color;
uniform samplerCube skybox;
void main()
{
color = texture(skybox, TexCoords);
}
)";

const char* shadowVertex = R"(#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}
)";

const char* shadowFragment = R"(#version 330 core
void main()
{
    // depth written automatically
}
)";

Engine::Engine(const char* title) {
	initWindow(title);

	glViewport(0, 0, SCR_W, SCR_H);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	engineViewMatrix = glm::lookAt(viewPos, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.0f, 1.0f, 0.0f));
	viewPos = glm::vec3(0.f);

	initSkybox();
	initShadowmap();

	initOpenAL();
}

void Engine::initSkybox() {
	// manual cubemap texture init, no api yet

	glGenTextures(1, &cubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

	const GLchar* faces[] = {
		"resources/skybox/right.jpg",
		"resources/skybox/left.jpg",
		"resources/skybox/top.jpg",
		"resources/skybox/bottom.jpg",
		"resources/skybox/front.jpg",
		"resources/skybox/back.jpg",
	};

	for (int i = 0; i < 6; i++) {
		int w, h, c;
		unsigned char* image = stbi_load(faces[i], &w, &h, &c, 3);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		stbi_image_free(image);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	skyboxShader = new Shader(skyVertex, skyFragment);

	unsigned int skyboxVBO;

	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);

	glBindVertexArray(skyboxVAO);

	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(skyboxVertices),
		&skyboxVertices,
		GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,              // location
		3,              // vec3
		GL_FLOAT,
		GL_FALSE,
		3 * sizeof(float),
		(void*)0
	);

	glBindVertexArray(0);
}

void Engine::initShadowmap() {
	glGenFramebuffers(1, &depthMapFBO);

	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	shadowShader = new Shader(shadowVertex, shadowFragment);
}

void Engine::initWindow(const char* title) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(SCR_W, SCR_H, title, NULL, NULL);
	if (!window) {
		return;
	}

	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		glfwTerminate();
		return;
	}

	glfwSwapInterval(1);

	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, Engine::onResize);
	glfwSetCursorPosCallback(window, Engine::onMouseCallback);
	glfwSetScrollCallback(window, Engine::onMouseScrollCallback);
}

void Engine::onResize(GLFWwindow* window, int w, int h) {
	Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
	if (engine) {
		engine->handleResize(w, h);
	}
}

void Engine::onMouseCallback(GLFWwindow* window, double mouseX, double mouseY) {
	Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
	if (engine && engine->mouse_callback) {
		engine->mouse_callback(window, mouseX, mouseY); // invoke user set callback
	}
}

void Engine::onMouseScrollCallback(GLFWwindow* window, double offsetX, double offsetY) {
	Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
	if (engine && engine->scroll_callback) {
		engine->scroll_callback(window, offsetX, offsetY); // invoke user set callback
	}
}

void Engine::handleResize(int w, int h) {
	SCR_W = w;
	SCR_H = h;
	glViewport(0, 0, SCR_W, SCR_H);
}

bool Engine::isRunning() {
	return !glfwWindowShouldClose(window);
}

glm::mat4 Engine::generateProjectionView(glm::mat4* viewPtr, glm::mat4* projectionPtr) {
	const glm::mat4 view = manualView ? engineViewMatrix : glm::lookAt(glm::vec3(0), viewPos, glm::vec3(0.0f, 1.0f, 0.0f));
	const glm::mat4 projection = glm::perspective(glm::radians(fov_deg), SCR_W / SCR_H, 0.1f, 100.f);

	if (viewPtr) 
		*viewPtr = view;
	if (projectionPtr) 
		*projectionPtr = projection;

	return projection * view;
}

void Engine::setViewMatrix(glm::mat4 view) {
	engineViewMatrix = view;
}

void Engine::renderSkybox() {
	glm::mat4 view, proj;
	generateProjectionView(&view, &proj);

	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);

	glDisable(GL_CULL_FACE);

	skyboxShader->use();
	skyboxShader->setMat4("view", glm::mat4(glm::mat3(view)));
	skyboxShader->setMat4("projection", proj);
	skyboxShader->setInt("skybox", 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

	glBindVertexArray(skyboxVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	glEnable(GL_CULL_FACE);

	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
}

void Engine::render(Shader& shader, Light& light) {
	glm::mat4 pv = generateProjectionView();

	// render shadows
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	constexpr float near_plane = 1.0f, far_plane = 15.0f;
	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	glm::mat4 lightView = glm::lookAt(light.pos,
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 lightSpaceMatrix = lightProjection * lightView;
	shadowShader->use();
	shadowShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
	for (auto& object : gameObjects) {
		if (!object.drawShadow) continue;
		object.Draw(pv, *shadowShader);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// render normally
	glViewport(0, 0, SCR_W, SCR_H);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	shader.use();
	shader.setVec3("lightColor", light.color);
	shader.setVec3("lightPos", light.pos);
	shader.setInt("shadowMap", 1);
	shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
	for (auto& object : gameObjects) {
		object.Draw(pv, shader);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Engine::registerGameObject(GameObject object) {
	gameObjects.push_back(object);
}

void Engine::clear() {
	float currentTime = glfwGetTime();
	deltaTime = currentTime - previousTimeStamp;
	previousTimeStamp = currentTime;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

GLFWwindow* Engine::getWindow() {
	return window;
}

void Engine::initOpenAL() {
	const char* kate = "resources/crabrave.wav"; // running up that hill yay
	const char* wind = "resources/ambient.wav";

	device = alcOpenDevice(nullptr);
	if(!device) {
		printf("OpenAL: Failed to open device\n");
		return;
	}

	const ALCint attrs[] = {
		ALC_HRTF_SOFT, ALC_TRUE,
		0
	};

	context = alcCreateContext(device,attrs);
	if(!context || !alcMakeContextCurrent(context)) {
		printf("OpenAL: Failed to create context\n");
		return;
	}

	ALCint hrtf = 0;
	alcGetIntegerv(device, ALC_HRTF_SOFT, 1, &hrtf);
	if(hrtf == 0) {
		// not fatal
		printf("OpenAL: HRTF not active!");
	}
}

ALuint Engine::registerAudioSource(const AudioSourceProperties& props) {
	ALuint buffer = loadAudioBuffer(props.audioFile);
	audioBuffers.push_back(buffer);
	ALuint source = createAudioSource(props, buffer);
	audioSources.push_back(source);
	return source;
}

ALuint Engine::createAudioSource(const AudioSourceProperties& props, const ALuint& buffer) {
	ALuint source;
	alGenSources(1, &source);
	alSourcei(source, AL_BUFFER, buffer);
	alSourcei(source, AL_LOOPING, props.looping ? AL_TRUE : AL_FALSE);
	alSourcei(source, AL_SOURCE_RELATIVE, props.relative ? AL_TRUE : AL_FALSE);
	alSourcef(source, AL_GAIN, props.gain);

    if(props.relative) {
		// non spatial
		alSource3f(source, AL_POSITION, 0.f,0.f,0.f);
	} else {
		// spatial
		alSourcef(source, AL_REFERENCE_DISTANCE, props.spatialProps.referenceDistance);
    	alSourcef(source, AL_ROLLOFF_FACTOR, props.spatialProps.rolloffFactor);
		glm::vec3 pos = props.spatialProps.position;
		alSource3f(source, AL_POSITION, pos.x, pos.y, pos.z);
	}
	
	return source;
}

ALuint Engine::loadAudioBuffer(const char* filename) {
    SF_INFO info;
    SNDFILE* file = sf_open(filename, SFM_READ, &info);
    if (!file) {
        printf("Failed to open audio file: %s\n", filename);
        return 0;
    }

    std::vector<short> samples(info.frames * info.channels);
    sf_readf_short(file, samples.data(), info.frames);
    sf_close(file);

    ALenum format;
    if (info.channels == 1) format = AL_FORMAT_MONO16;
    else if (info.channels == 2) format = AL_FORMAT_STEREO16;
    else return 0;

    ALuint newBuffer;
    alGenBuffers(1, &newBuffer);
    alBufferData(newBuffer, format, samples.data(), samples.size() * sizeof(short), info.samplerate);
    
    return newBuffer;
}

void Engine::destroyAudioSource(ALuint source) {
	alDeleteSources(1, &source);
}

void Engine::destroyAudioBuffer(ALuint buffer) {
	alDeleteBuffers(1, &buffer);
}

void Engine::cleanupOpenAL() {
	for(auto& buffer: audioBuffers) {
		destroyAudioBuffer(buffer);
	}

	for(auto& source: audioSources) {
		destroyAudioSource(source);
	}

	alcMakeContextCurrent(nullptr);
	alcDestroyContext(context);
	alcCloseDevice(device);
}

void Engine::terminate() {
	cleanupOpenAL();
	glfwTerminate();
}

void Engine::startPlayback() {
	for(auto& source : audioSources) {
		alSourcePlay(source);
	}
}

void Engine::setListener(glm::vec3 camPos, glm::vec3 camFront, glm::vec3 worldUp) {
    alListener3f(AL_POSITION, camPos.x, camPos.y, camPos.z);

    glm::vec3 right = glm::normalize(glm::cross(camFront, worldUp));
    glm::vec3 trueUp = glm::normalize(glm::cross(right, camFront));
    float orientation[] = { camFront.x, camFront.y, camFront.z, trueUp.x, trueUp.y, trueUp.z };
    alListenerfv(AL_ORIENTATION, orientation);
}
