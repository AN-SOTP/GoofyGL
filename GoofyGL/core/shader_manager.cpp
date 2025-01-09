#include "../core/shader_manager.h"

Shader::Shader()
{

}

Shader::Shader(const char* vertex_path, const char* fragement_path)
{
	std::string vertex_code;
	std::string fragement_code;
	std::ifstream v_shader_file;
	std::ifstream f_shader_file;

	//ensure ifstream objects can throw exceptions
	v_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	f_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		//open files
		v_shader_file.open(vertex_path);
		f_shader_file.open(fragement_path);
		std::stringstream v_shader_stream, f_shader_stream;
		//read file's buffer contents into streams
		v_shader_stream << v_shader_file.rdbuf();
		f_shader_stream << f_shader_file.rdbuf();
		//close file handlers
		v_shader_file.close();
		f_shader_file.close();
		//convert stream into string
		vertex_code = v_shader_stream.str();
		fragement_code = f_shader_stream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
	}
	const char* v_shader_code = vertex_code.c_str();
	const char* f_shader_code = fragement_code.c_str();
	//compile shaders
	//unsigned int vertex, fragment;
	int success;
	char info_log[512];

	//vertex shader
	GLuint vertex;
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &v_shader_code, NULL);
	glCompileShader(vertex);
	//check for shader compilation errors, print if any
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, NULL, info_log);
		std::cout << "ERROR::SHADER::VERTEX::COMPILTION_FAILED\n" << info_log << std::endl;
	}

	//fragment shader
	GLuint fragment;
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &f_shader_code, NULL);
	glCompileShader(fragment);
	//check for shader compilation errors, print if any
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment, 512, NULL, info_log);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILTION_FAILED\n" << info_log << std::endl;
	}

	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);
	//print linking error if any
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(ID, 512, NULL, info_log);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info_log << std::endl;
	}

	//delete shaders as they are linked into the program and now longer needed
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Shader::Use()
{
	glUseProgram(ID);
}

void Shader::SetBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::SetInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::SetFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::SetMat4(const std::string& name, const glm::mat4& mat) const
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::SetVec3(const std::string& name, const glm::vec3& vec) const
{
	glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &vec[0]);
}

void Shader::SetUint64(const std::string& name, uint64_t value) const
{
	//GLint location = glGetUniformLocation(ID, name.c_str());
	glUniformHandleui64ARB(glGetUniformLocation(ID, name.c_str()), value);
}