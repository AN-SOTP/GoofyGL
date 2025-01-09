#ifndef GOOFYGL_H
#define GOOFYGL_H

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include"imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "shader_manager.h"
#include "../Cameras/Header/Camera.h"
#include "../external/stb_image.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "../Cameras/Header/ExampleCamera.h"
#include "../graphics/Model.h"

#ifdef _WIN32
#include <windows.h>
#include <pdh.h>
#pragma comment(lib, "pdh.lib")
#endif


class GoofyGL
{
public:
	GoofyGL();
	~GoofyGL();
	//int GoofyGLInit();
	void GoofyGLRun();
	
	//timing
	float delta_time = 0.0f; //time between current and last frame
	float last_frame = 0.0f;
	glm::vec3 light_pos;
	const unsigned int screen_width = 1280;
	const unsigned int screen_height = 720;
	Camera main_camera;
	bool cursor_enabled = false;
	bool first_mouse = true;
	float last_x = screen_width / 2;
	float last_y = screen_height / 2;
private:
	GLFWwindow* window;
	static void FramebufferSizeCallback(GLFWwindow* _window, int _width, int _height);
	static void MouseCallback(GLFWwindow* _window, double _xpos, double _ypos);
	static void ScrollCallback(GLFWwindow* _window, double _xoffset, double _yoffset);
	void ProcessInput(GLFWwindow* _window);
	unsigned int LoadTexture(char const* path);

	//fps and performance shit
	float fps = 0.0f;
	float frame_time = 0.0f;
	float cpu_usage = 0.0f;
	//float gpu_usage = 0.0f;

#ifdef _WIN32
	PDH_HQUERY cpu_query;
	PDH_HCOUNTER cpu_counter;
#endif

	void InitPerformanceStats();
	void UpdatePerformanceStats();

	std::vector<float> GenerateGridVertices(float grid_size, float grid_spacing);
	void GenerateGrid();
	unsigned int grid_VAO, grid_VBO;
	//grid values
	float grid_size = 100.0f;
	float grid_spacing = 1.0f;
	Shader grid_shader;

	bool vertical_sync = true;

	bool visualize_depth = false;
	bool wireframe_mode = false;
};

#endif