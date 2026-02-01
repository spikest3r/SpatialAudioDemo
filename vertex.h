#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "shader.h"

using namespace std;

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords; // optional
};

class Mesh {
public:
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	Mesh() {};
	Mesh(vector<Vertex> vertices, vector<unsigned int> indices);
	void Draw(Shader& shader);
private:
	unsigned int VAO, VBO, EBO;
	void setupMesh();
};

class Model {
public:
	Model(const char* path) {
		loadModel(path);
	}
	Model() {};
	vector<Mesh> meshes;
	void loadModel(const string& path);
	void Draw(Shader& shader);
private:
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	string directory;
};