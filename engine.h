#pragma once
#include <functional>
#include <sndfile.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

struct Light {
	glm::vec3 pos;
	glm::vec3 color;
};

struct SpatialProperties {
	glm::vec3 position;
	float referenceDistance;
	float rolloffFactor;
};

struct AudioSourceProperties{
	const char* audioFile;

	bool relative;
	bool looping;
	float gain;

	SpatialProperties spatialProps;
};

class Engine {
public:
	Engine(const char* title = "Engine");
	bool isRunning();

	glm::mat4 generateProjectionView(glm::mat4* viewPtr = nullptr, glm::mat4* projectionPtr = nullptr);

	void render(Shader& shader, Light& light);
	void renderSkybox();
	void clear();
	void registerGameObject(GameObject object);

	void terminate();

	void startPlayback();
	void setListener(glm::vec3 camPos, glm::vec3 camFront, glm::vec3 camUp);

	ALuint registerAudioSource(const AudioSourceProperties& props);

	GLFWwindow* getWindow();

	// camera settings
	glm::vec3 viewPos = glm::vec3(8.0f, 9.0f, 8.0f);
	void setViewMatrix(glm::mat4 view);

	std::function<void(GLFWwindow*, double, double)> mouse_callback;
	std::function<void(GLFWwindow*, double, double)> scroll_callback;

	float fov_deg = 45.0f;

	float deltaTime = 0.0f;

	bool manualView = false;
private:
	float SCR_W = 800;
	float SCR_H = 800;

	static void onResize(GLFWwindow* window, int w, int h);
	static void onMouseCallback(GLFWwindow* window, double mouseX, double mouseY);
	static void onMouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
	void handleResize(int w, int h);

	ALuint loadAudioBuffer(const char* filename);
	ALuint createAudioSource(const AudioSourceProperties& props, const ALuint& buffer);
	void destroyAudioSource(ALuint source);
	void destroyAudioBuffer(ALuint buffer);

	GLFWwindow* window;

	void initWindow(const char* title);
	void initSkybox();
	void initShadowmap();
	Shader* skyboxShader;
	Shader* shadowShader;
	unsigned int cubemap;
    unsigned int skyboxVAO;
	unsigned int depthMapFBO;
	unsigned int depthMap;
	const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

	std::vector<GameObject> gameObjects;
	std::vector<ALuint> audioSources;
	std::vector<ALuint> audioBuffers;

	glm::mat4 engineViewMatrix;

	// delta time
	float previousTimeStamp;

	//audio
	void initOpenAL();
	void cleanupOpenAL();
	
	ALCdevice* device;
	ALCcontext* context;
};