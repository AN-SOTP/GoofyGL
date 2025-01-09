#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include <glad/glad.h>
#include "glm/glm.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
	//the program ID
	unsigned int ID;

	Shader();
	//constructor to read and build shader
	Shader(const char* vertex_path, const char* fragement_path);
	//use/activate the shader
	void Use();
	//utility uniform functions
	void SetBool(const std::string& name, bool value) const;
	void SetInt(const std::string& name, int value) const;
	void SetFloat(const std::string& name, float value) const;
	void SetMat4(const std::string& name, const glm::mat4& mat) const;
	void SetVec3(const std::string& name, const glm::vec3& vec) const;
	void SetUint64(const std::string& name, uint64_t value) const;
};

#endif