#include "gameobject.h"
#include "texture.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad.h>

GameObject::GameObject(Model* m, const char* textureFile) {
	model = *m;

	texture = Texture(textureFile);

	init();
}

GameObject::GameObject(Mesh* mesh, const char* textureFile) {
	Model m;
	m.meshes.push_back(*mesh);
	model = m;

	texture = Texture(textureFile);

	init();
}

void GameObject::SetTransform(glm::vec3 p, glm::vec3 r, glm::vec3 s) {
	position = p;
	rotation = r;
	scale = s;
}

void GameObject::init() {
	SetTransform(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));

	// TODO: Create initialization logic (e.g. PhysX init etc.)
}

void GameObject::Draw(glm::mat4 pv, Shader& shader) {
    glm::mat4 objectModel(1.0f);
	objectModel = glm::translate(objectModel, position);
	objectModel = glm::rotate(objectModel, glm::radians(rotation.x), glm::vec3(1, 0, 0));
	objectModel = glm::rotate(objectModel, glm::radians(rotation.y), glm::vec3(0, 1, 0));
	objectModel = glm::rotate(objectModel, glm::radians(rotation.z), glm::vec3(0, 0, 1));
	objectModel = glm::scale(objectModel, scale);

	shader.setMat4("mvp", pv * objectModel);
	shader.setMat4("model", objectModel);
	shader.setInt("tex", 0);
	texture.use(GL_TEXTURE0);
	model.Draw(shader); // shader is unused here for now
}