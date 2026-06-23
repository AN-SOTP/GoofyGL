#include "GoofyGL.h"

GoofyGL::GoofyGL()
{
	main_camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), base_yaw, base_pitch);
	light_pos = glm::vec3(1.2f, 1.0f, 2.0f);


	//glfw initialisation and configuration
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	//glfw window creation
	window = glfwCreateWindow(screen_width, screen_height, "GoofyGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		//return -1;
	}
	glfwSetWindowUserPointer(window, reinterpret_cast<void*>(this));
	//GoofyGL* obj = reinterpret_cast<GoofyGL*>(glfwGetWindowUserPointer(window));
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, MouseCallback);
	glfwSetScrollCallback(window, ScrollCallback);
	glfwSwapInterval(vertical_sync); //enable vsync

	//glad, loading all opengl function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialise GLAD" << std::endl;
		//return -1;
	}

	if (!GL_ARB_bindless_texture)
	{
		//might mess around with this soon
		std::cerr << "Bindless textures not supported on this system. Whoops!" << std::endl;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_CLAMP);

	glEnable(GL_STENCIL_TEST);

	// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	stbi_set_flip_vertically_on_load(true);

	InitPerformanceStats();
	//return 0;
}

GoofyGL::~GoofyGL()
{
	
}

/*int GoofyGL::GoofyGLInit()
{
	return 0;
}*/

void GoofyGL::GoofyGLRun()
{
	//build and compile shaders
	//------------------------
	//Shader first_shader("shader.vs", "shader.fs");
	Shader lighting_shader("assets/shaders/light_shader.vs", "assets/shaders/light_shader.fs");
	//Shader light_cube_shader("assets/shaders/light_cube.vs", "assets/shaders/light_cube.fs");
	//Shader model_loading_shader("assets/shaders/model_test_shader.vs", "assets/shaders/model_test_shader.fs");

	//load models
	//Model first_model("assets/models/backpack/backpack.obj");
	//Model first_model("assets/models/cottage/cottage_obj.obj");
	//Model first_model("assets/models/car/mustang.obj");
	Model first_model("assets/models/Sponza-master/sponza.obj");
	Model second_model("assets/models/backpack/backpack.obj");

	//set up vertex data and buffers and configure vertex attributes
	//glViewport(0, 0, 800, 600);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	GenerateGrid();
	
	//imgui test
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430");

	//variables to manipulate in imgui window
	glm::vec3 point_lights_ambient = glm::vec3(0.05f, 0.05f, 0.05f);
	glm::vec3 point_lights_diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
	glm::vec3 point_lights_specular = glm::vec3(1.0f, 1.0f, 1.0f);
	float point_lights_constant = 1.0f;
	float point_lights_linear = 0.09f;
	float point_lights_quadratic = 0.032f;

	glm::vec3 point_lights_positions[] = {
	glm::vec3(-45.f,  109.f,  -3.f),
	glm::vec3(2.3f, -3.3f, -4.0f),
	glm::vec3(-4.0f,  2.0f, -12.0f),
	glm::vec3(0.0f,  0.0f, -3.0f)
	};

	//glDisable(GL_CULL_FACE);
	//glFrontFace(GL_CCW);

	while (!glfwWindowShouldClose(window))
	{
		float current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;

		//input (also applies the cursor mode change when TAB toggles cursor_enabled)
		ProcessInput(window);

		//rendering
		glClearColor(0.00f, 0.00f, 0.00f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		//model_loading_shader.Use();

		//tell opengl a new frame is about to begin
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		//can just use same vertex shader but whatever
		grid_shader.Use();
		//set transform shit
		glm::mat4 grid_projection = glm::perspective(glm::radians(main_camera.zoom), (float)screen_width / (float)screen_height, 0.1f, 10000.0f);
		glm::mat4 grid_view = main_camera.GetViewMatrix();
		grid_shader.SetMat4("projection", grid_projection);
		grid_shader.SetMat4("view", grid_view);

		//center grid around camera
		glm::vec3 camera_position = main_camera.position;
		glm::mat4 grid_model = glm::mat4(1.0f);
		grid_model = glm::translate(grid_model, glm::vec3(floor(camera_position.x / grid_spacing) * grid_spacing, 0.0f, floor(camera_position.z / grid_spacing) * grid_spacing));
		grid_shader.SetMat4("model", grid_model);

		//set grid colour to white
		grid_shader.SetVec3("grid_color", glm::vec3(1.0f, 1.0f, 1.0f));

		glDepthMask(GL_FALSE);

		//bind and draw grid
		glBindVertexArray(grid_VAO);
		int num_of_lines = static_cast<int>((2 * grid_size / grid_spacing) + 1) * 2; //lines parallel to both axes
		glDrawArrays(GL_LINES, 0, num_of_lines * 2); //each line has 2 vertices
		glBindVertexArray(0);

		glDepthMask(GL_TRUE);

		lighting_shader.Use();

		lighting_shader.SetVec3("view_pos", main_camera.position);

		//set values of properties in the different light structs
		//directional light
		lighting_shader.SetVec3("directional_light.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
		lighting_shader.SetVec3("directional_light.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
		lighting_shader.SetVec3("directional_light.diffuse", glm::vec3(0.4f, 0.4f, 0.4f));
		lighting_shader.SetVec3("directional_light.specular", glm::vec3(0.5f, 0.5f, 0.5f));

		//looping through the four point lights and setting their values
		for (int i = 0; i < 4; i++)
		{
			std::string number = std::to_string(i);
			lighting_shader.SetVec3("point_lights[" + number + "].position", point_lights_positions[i]);
			lighting_shader.SetVec3("point_lights[" + number + "].ambient", point_lights_ambient);
			lighting_shader.SetVec3("point_lights[" + number + "].diffuse", point_lights_diffuse);
			lighting_shader.SetVec3("point_lights[" + number + "].specular", point_lights_specular);
			lighting_shader.SetFloat("point_lights[" + number + "].constant", point_lights_constant);
			lighting_shader.SetFloat("point_lights[" + number + "].linear", point_lights_linear);
			lighting_shader.SetFloat("point_lights[" + number + "].quadratic", point_lights_quadratic);
		}

		//spotlight
		
		lighting_shader.SetVec3("spot_light.position", main_camera.position);
		lighting_shader.SetVec3("spot_light.direction", main_camera.front);
		lighting_shader.SetVec3("spot_light.ambient", glm::vec3(0.00f, 0.00f, 0.00f));
		lighting_shader.SetVec3("spot_light.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
		lighting_shader.SetVec3("spot_light.specular", glm::vec3(1.0f, 1.0f, 1.0f));
		lighting_shader.SetFloat("spot_light.constant", 1.0f);
		lighting_shader.SetFloat("spot_light.linear", 0.09f);
		lighting_shader.SetFloat("spot_light.quadratic", 0.032f);
		lighting_shader.SetFloat("spot_light.cutoff", glm::cos(glm::radians(12.5f)));
		lighting_shader.SetFloat("spot_light.outer_cutoff", glm::cos(glm::radians(15.0f)));

		//sampler bindings and per-mesh material values are set inside Mesh::Draw.
		//(previously we set "material.diffuse"/"material.specular" here, but those names
		//don't exist in the shader — the samplers are "material_diffuse"/"material_specular",
		//and the struct field "material.diffuse" is the vec3 color, not a sampler.)

		glm::mat4 projection = glm::perspective(glm::radians(main_camera.zoom), (float)screen_width / (float)screen_height, 0.1f, 1000.0f);
		glm::mat4 view = main_camera.GetViewMatrix();
		lighting_shader.SetMat4("projection", projection);
		lighting_shader.SetMat4("view", view);

		lighting_shader.SetBool("wireframe", wireframe_mode);
		lighting_shader.SetBool("visualize_depth", visualize_depth);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it so it's at the center of the scene
		//model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// model is a bit too big for the scene, so scale it down
		model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
		lighting_shader.SetMat4("model", model);
		first_model.Draw(lighting_shader);
		model = glm::translate(model, glm::vec3(-70.0f, 50.0, 0.0f));
		model = glm::scale(model, glm::vec3(20.0f, 20.0f, 20.0f));
		lighting_shader.SetMat4("model", model);
		second_model.Draw(lighting_shader);

		//glBindVertexArray(0); //no need to unbind everytime
		//check and poll IO events and swap front and back buffer

		if (wireframe_mode)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		UpdatePerformanceStats();

		//imgui window creation, name and features (text, checkbox etc.)
		ImGui::Begin("Test Window");
		ImGui::Text("Hello GoofyGL");
		if (ImGui::TreeNode("Performance"))
		{
			ImGui::Text("FPS: %.1f", fps);
			ImGui::Text("Frame time: %.2f ms", frame_time);
			ImGui::Text("CPU usage: %.1f%%", cpu_usage);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Other shit"))
		{
			ImGui::SliderFloat3("Point Lights ambient", glm::value_ptr(point_lights_ambient), 0.0f, 1.0f);
			ImGui::SliderFloat3("Point Lights diffuse", glm::value_ptr(point_lights_diffuse), 0.0f, 1.0f);
			ImGui::SliderFloat3("Point Lights specular", glm::value_ptr(point_lights_specular), 0.0f, 1.0f);
			ImGui::SliderFloat("Point Lights constant", &point_lights_constant, 0.0f, 1.0f);
			ImGui::SliderFloat("Point Lights linear", &point_lights_linear, 0.0f, 1.0f);
			ImGui::SliderFloat("Point Lights quadratic", &point_lights_quadratic, 0.0f, 1.0f);
			ImGui::Text("Camera position: (%.1f, %.1f, %.1f)", main_camera.position.x, main_camera.position.y, main_camera.position.z);
			ImGui::Checkbox("Wireframe mode", &wireframe_mode);
			ImGui::Checkbox("Visualize Depth", &visualize_depth);
			ImGui::TreePop();
		}
		ImGui::End();

		//render imgui elements
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//end imgui processes
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	//delete all created objects
	//glDeleteVertexArrays(1, &VAO);
	//glDeleteVertexArrays(1, &light_cubeVAO);
	//glDeleteBuffers(1, &VBO);
	//glDeleteProgram(shader_program);

	//glfwterminate clearing all previously allocated glfw resources
	glfwTerminate();
}

//glfw, whenever the window size changed this callback function executes
void GoofyGL::FramebufferSizeCallback(GLFWwindow* _window, int _width, int _height)
{
	glViewport(0, 0, _width, _height);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void GoofyGL::ProcessInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	//edge-detect TAB: only flip on the RELEASE -> PRESS transition, otherwise the toggle
	//fires every frame the key is held and cursor_enabled flickers (visually it looks random
	//whether the cursor ends up shown or hidden, depending on parity of frames during the tap).
	bool tab_is_pressed = glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS;
	if (tab_is_pressed && !tab_was_pressed)
	{
		cursor_enabled = !cursor_enabled;
		glfwSetInputMode(window, GLFW_CURSOR, cursor_enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
		//reset first_mouse so re-grabbing the cursor doesn't produce a giant jump on the next mouse event
		if (!cursor_enabled)
			first_mouse = true;
	}
	tab_was_pressed = tab_is_pressed;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		main_camera.ProcessKeyboard(FORWARD, delta_time);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		main_camera.ProcessKeyboard(BACKWARD, delta_time);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		main_camera.ProcessKeyboard(LEFT, delta_time);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		main_camera.ProcessKeyboard(RIGHT, delta_time);
}


void GoofyGL::MouseCallback(GLFWwindow* _window, double _xpos, double _ypos)
{
	//GoofyGL* GoofyGL = reinterpret_cast<GoofyGL*>(glfwGetWindowUserPointer(_window));
	//GoofyGL goofy;

	GoofyGL* GoofyGLInstance = static_cast<GoofyGL*>(glfwGetWindowUserPointer(_window));
	if (!GoofyGLInstance->cursor_enabled)
	{
		float x_pos = static_cast<float>(_xpos);
		float y_pos = static_cast<float>(_ypos);

		if (GoofyGLInstance->first_mouse)
		{
			GoofyGLInstance->last_x = x_pos;
			GoofyGLInstance->last_y = y_pos;
			GoofyGLInstance->first_mouse = false;
		}

		float x_offset = x_pos - GoofyGLInstance->last_x;
		float y_offset = GoofyGLInstance->last_y - y_pos; //reversed since y-coordinates range from bottom to top

		GoofyGLInstance->last_x = x_pos;
		GoofyGLInstance->last_y = y_pos;

		GoofyGLInstance->main_camera.ProcessMouseMovement(x_offset, y_offset);
	}
}

void GoofyGL::ScrollCallback(GLFWwindow* _window, double _xoffset, double _yoffset)
{
	GoofyGL* GoofyGLInstance = static_cast<GoofyGL*>(glfwGetWindowUserPointer(_window));
	if (!GoofyGLInstance->cursor_enabled)
	{
		GoofyGLInstance->main_camera.ProcessMouseScroll(static_cast<float>(_yoffset));
	}
}

unsigned int GoofyGL::LoadTexture(char const * path)
{
	unsigned int texture_id;
	glGenTextures(1, &texture_id);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, texture_id);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return texture_id;
}

void GoofyGL::InitPerformanceStats()
{
	#ifdef _WIN32
	//PdhAddCounter requires a query handle created by PdhOpenQuery first;
	//without it cpu_query is uninitialised and the counter add silently fails.
	if (PdhOpenQuery(NULL, 0, &cpu_query) != ERROR_SUCCESS)
	{
		std::cerr << "PdhOpenQuery failed; CPU usage will not be available." << std::endl;
		cpu_query = NULL;
		return;
	}
	std::wstring cpu_counter_path = L"\\Processor(_Total)\\% processor time";
	PdhAddCounter(cpu_query, cpu_counter_path.c_str(), NULL, &cpu_counter);
	//first PdhCollectQueryData primes the counter; the first formatted read after this returns 0,
	//so real values start arriving on the second collect (in UpdatePerformanceStats).
	PdhCollectQueryData(cpu_query);
	#endif
}

void GoofyGL::UpdatePerformanceStats()
{
	//update fps and frame tim
	static int frame_count = 0;
	static float time_sum = 0.0f;
	static float last_update = glfwGetTime();

	float current_time = glfwGetTime();
	frame_count++;
	time_sum += delta_time;

	if (current_time - last_update >= 1.0f)
	{
		fps = static_cast<float>(frame_count);
		frame_time = (time_sum * 1000.0f) / frame_count; // Convert to milliseconds

		frame_count = 0;
		time_sum = 0.0f;
		last_update = current_time;

		// Update CPU usage
		#ifdef _WIN32
		if (cpu_query)
		{
			PDH_FMT_COUNTERVALUE counterVal;
			PdhCollectQueryData(cpu_query);
			if (PdhGetFormattedCounterValue(cpu_counter, PDH_FMT_DOUBLE, NULL, &counterVal) == ERROR_SUCCESS)
				cpu_usage = static_cast<float>(counterVal.doubleValue);
		}
		#endif
	}
}


std::vector<float> GoofyGL::GenerateGridVertices(float grid_size, float grid_spacing)
{
	std::vector<float> vertices;

	for (float i = -grid_size; i <= grid_size; i += grid_spacing) {
		//lines parallel to Z-axis
		vertices.push_back(i);
		vertices.push_back(0.0f);
		vertices.push_back(-grid_size);

		vertices.push_back(i);
		vertices.push_back(0.0f);
		vertices.push_back(grid_size);

		//lines parallel to X-axis
		vertices.push_back(-grid_size);
		vertices.push_back(0.0f);
		vertices.push_back(i);

		vertices.push_back(grid_size);
		vertices.push_back(0.0f);
		vertices.push_back(i);
	}

	return vertices;
}

void GoofyGL::GenerateGrid()
{
	grid_size = 5000.0f;
	grid_spacing = 25.0f;
	std::vector<float> grid_vertices = GenerateGridVertices(grid_size, grid_spacing);

	glGenVertexArrays(1, &grid_VAO);
	glGenBuffers(1, &grid_VBO);

	glBindVertexArray(grid_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, grid_VBO);
	glBufferData(GL_ARRAY_BUFFER, grid_vertices.size() * sizeof(float), &grid_vertices[0], GL_STATIC_DRAW);

	//position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	grid_shader = Shader("assets/shaders/grid_shader.vs", "assets/shaders/grid_shader.fs");
}
