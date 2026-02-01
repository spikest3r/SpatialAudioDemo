#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "vertex.h"
#include "texture.h"
#include "shader.h"

class GameObject {
public:
	GameObject(Model* model, const char* textureFile);
	GameObject(Mesh* mesh, const char* textureFile);
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	void Draw(glm::mat4 pv, Shader& shader);
	void SetTransform(glm::vec3 p, glm::vec3 r = glm::vec3(0.f), glm::vec3 s = glm::vec3(1.0f));
	bool drawShadow = true;
private:
	Model model;
	Texture texture;
	void init(); // same for both constructors
};