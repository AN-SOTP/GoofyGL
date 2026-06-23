#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/shader_manager.h"
#include "Material.h"

#include <string>
#include <vector>

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 tex_coords;
};

struct Texture {
	unsigned int id;
	std::string type; //diffuse, specular etc.
	std::string path;
};

class Mesh
{
public:
	//mesh data
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	Material material;

	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, const Material& material)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;
		this->material = material;

		SetupMesh();
	}

	void Draw(Shader& shader)
	{
		//shader exposes a single sampler for each map type (material_diffuse / material_specular).
		//bind the first texture of each type we find; any extras are ignored.
		bool diffuse_bound = false;
		bool specular_bound = false;
		for (unsigned int i = 0; i < textures.size(); i++)
		{
			const std::string& name = textures[i].type;
			if (name == "texture_diffuse" && !diffuse_bound)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, textures[i].id);
				shader.SetInt("material_diffuse", 0);
				diffuse_bound = true;
			}
			else if (name == "texture_specular" && !specular_bound)
			{
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, textures[i].id);
				shader.SetInt("material_specular", 1);
				specular_bound = true;
			}
		}
		glActiveTexture(GL_TEXTURE0);

		shader.SetVec3("material.diffuse_color", material.diffuse);
		shader.SetVec3("material.ambient_color", material.ambient);
		shader.SetVec3("material.specular_color", material.specular);
		shader.SetFloat("material.shininess", material.shininess);

		//draw the mesh
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

private:
	//render data
	unsigned int VAO, VBO, EBO;
	void SetupMesh()
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		//vertex positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		//vertex normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		//vertex texture coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));
	}
};

#endif