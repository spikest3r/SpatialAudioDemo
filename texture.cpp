#include "texture.h"
#include <glad.h>
#include "stb_image.h"

Texture::Texture(const char* fileName, bool* success) {
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);

	int w, h, c;
	unsigned char* chair_image = stbi_load(fileName, &w, &h, &c, 4);
	if (!chair_image) {
		if (success) *success = false;
		return;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, chair_image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(chair_image);

	if (success) *success = true;
}

void Texture::use(unsigned int texUnit) {
	glActiveTexture(texUnit);
	glBindTexture(GL_TEXTURE_2D, ID);
}