#include "../core/shader_manager.h"
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../external/stb_image.h"
#include <unordered_map>
#include <chrono>

#ifndef MODEL_H
#define MODEL_H

class Model
{
public:

	Model(const std::string &path)
	{
		LoadModel(path);
	}
	void Draw(Shader& shader)
	{
		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			meshes[i].Draw(shader);
		}
	}
private:
	//model data
	std::vector<Mesh> meshes;
	std::string directory;
	//std::vector<Texture> loaded_textures;
	std::unordered_map<std::string, Texture> texture_cache;

	void LoadModel(std::string path)
	{
		std::cout << "Starting model load..." << std::endl;

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, 
			aiProcess_Triangulate 
			| aiProcess_FlipUVs
			//|aiProcess_OptimizeMeshes //further optimization after FlipUVs, can disable for performance
			//|aiProcess_OptimizeGraph //can disable for performance
			//|aiProcess_JoinIdenticalVertices //can disable for performance
			//|aiProcess_ValidateDataStructure //can disable for performance
			|aiProcess_GenSmoothNormals);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
			return;
		}
		directory = path.substr(0, path.find_last_of('/'));

		ProcessNode(scene->mRootNode, scene);
	}
	void ProcessNode(aiNode* node, const aiScene* scene)
	{
		meshes.reserve(node->mNumMeshes);
		//process all the node's meshes
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(ProcessMesh(mesh, scene));
		}
		//do the same for its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene);
		}
	}

	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<Vertex> vertices;
		vertices.reserve(mesh->mNumVertices);
		std::vector<unsigned int> indices;
		indices.reserve(mesh->mNumFaces * 3); //triangulated faces due to assimp triangulate flag
		std::vector<Texture> textures;
		Material material_data;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			glm::vec3 vector;
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.position = vector;

			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.normal = vector;

			if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.tex_coords = vec;
			}
			else
			{
				vertex.tex_coords = glm::vec2(0.0f, 0.0f);
			}
			
			vertices.push_back(vertex);
		}
		//go through each face (which are triangles due to aiProcess_Triangulate) and store the indices in the indices vector
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		//mesh contains index to a material object. to get the material of a mesh we index the scene's mMaterials array.
		//the mesh's material index is set in its mMaterialIndex property which we can query to check if the mesh contains a material
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			//load diffuse textures
			std::vector<Texture> diffuse_maps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());
			//load specular textures
			std::vector<Texture> specular_maps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());

			// load color-based material data
			material_data = LoadMaterial(material);
		}
		else
		{
			material_data.ambient = glm::vec3(0.2f);
			material_data.diffuse = glm::vec3(0.8f);
			material_data.specular = glm::vec3(1.0f);
			material_data.shininess = 32.0f;
		}

		std::cout << "Vertices: " << vertices.size() << ", Indices: " << indices.size() << std::endl;

		/*std::cout << "Vertices:" << std::endl;
		for (size_t i = 0; i < vertices.size(); ++i) {
			std::cout << "Position: " << vertices[i].position.x << ", "
				<< vertices[i].position.y << ", " << vertices[i].position.z << std::endl;
		}

		std::cout << "Indices:" << std::endl;
		for (size_t i = 0; i < indices.size(); ++i) {
			std::cout << indices[i] << " ";
		}
		std::cout << std::endl;*/

		return Mesh(vertices, indices, textures, material_data);
	}

	std::vector<Texture> LoadMaterialTextures(aiMaterial* material, aiTextureType type, std::string type_name)
	{
		std::vector<Texture> textures;
		unsigned int texture_count = material->GetTextureCount(type);
		for (unsigned int i = 0; i < texture_count; i++)
		{
			aiString str;
			material->GetTexture(type, i, &str);
			std::string full_path = directory + '/' + str.C_Str();

			//check texture cache
			auto cached = texture_cache.find(full_path);
			if (cached != texture_cache.end())
			{
				textures.push_back(cached->second);
				std::cout << "Using cached texture: " << full_path << std::endl;
				continue;
			}

			//if not found load new texture
			Texture texture;
			texture.id = TextureFromFile(str.C_Str(), directory);
			texture.type = type_name;
			texture.path = str.C_Str();
			textures.push_back(texture);
			texture_cache[full_path] = texture;

			std::cout << "Loading new texture: " << full_path << std::endl;
		}

		//if no textures
		if (texture_count == 0)
		{
			unsigned int white_texture = CreateWhiteTexture();
			Texture fallback_texture;
			fallback_texture.id = white_texture;
			fallback_texture.type = type_name;
			fallback_texture.path = "fallback_white";
			textures.push_back(fallback_texture);
		}

		return textures;
	}
	
	//uses stb_image to load a texture
	unsigned int TextureFromFile(const char* path, const std::string& directory)
	{
		std::string filename = directory + '/' + std::string(path);

		unsigned int textureID;
		glGenTextures(1, &textureID);

		int width, height, nrComponents;
		unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			GLenum format = nrComponents == 1 ? GL_RED :
				nrComponents == 3 ? GL_RGB :
				nrComponents == 4 ? GL_RGBA : GL_RGB;

			glBindTexture(GL_TEXTURE_2D, textureID);
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
			std::cout << "Texture failed to load at path: " << filename << std::endl;
			stbi_image_free(data);
			return 0;
		}

		return textureID;
	}
	
	Material LoadMaterial(aiMaterial* mat)
    {
        Material material;
        aiColor3D color(0.f, 0.f, 0.f);
        float shininess = 0.0f;

        // diffuse
        if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
            material.diffuse = glm::vec3(color.r, color.g, color.b);
        else
            material.diffuse = glm::vec3(0.8f);

        // ambient
        if (mat->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS)
            material.ambient = glm::vec3(color.r, color.g, color.b);
        else
            material.ambient = glm::vec3(0.2f);

        // specular
        if (mat->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS)
            material.specular = glm::vec3(color.r, color.g, color.b);
        else
            material.specular = glm::vec3(1.0f);

        // shininess
        if (mat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS)
            material.shininess = shininess;
        else
            material.shininess = 32.0f;

        return material;
    }

	static unsigned int CreateWhiteTexture()
	{
		static bool created = false;
		static unsigned int white_texture_id = 0;

		if (!created)
		{
			//a single white pixel: RGBA = 255,255,255,255
			unsigned char white_pixel[4] = { 255, 255, 255, 255 };

			glGenTextures(1, &white_texture_id);
			glBindTexture(GL_TEXTURE_2D, white_texture_id);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, white_pixel);

			glGenerateMipmap(GL_TEXTURE_2D);

			// set wrap/filter modes as you wish
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			created = true;
		}
		return white_texture_id;
	}
};
#endif