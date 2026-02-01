#pragma once
class Texture {
public:
	Texture(const char* fileName, bool* success = nullptr);
	Texture() {};
	void use(unsigned int texUnit);
private:
	unsigned int ID;
};