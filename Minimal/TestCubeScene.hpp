#pragma once

#include "Core.h"
#include "OvrHelper.h"

#include "BasicShader.h"
#include "Skybox.h"
#include <vector>

// a class for building and rendering cubes
class TestCubeScene
{
	// Program
	std::vector<glm::mat4> instance_positions;
	GLuint instanceCount;
	GLuint shaderID;

	std::unique_ptr<TexturedCube> cube;
	std::unique_ptr<Skybox> skybox;

	const unsigned int GRID_SIZE{ 5 };

public:
	TestCubeScene()
	{
		// Create two cube
		instance_positions.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -0.3)));
		instance_positions.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -0.9)));

		instanceCount = instance_positions.size();

		// Shader Program 
		shaderID = LoadShaders("skybox.vert", "skybox.frag");

		cube = std::make_unique<TexturedCube>("cube");

		// 10m wide sky box: size doesn't matter though
		skybox = std::make_unique<Skybox>("skybox");
		skybox->toWorld = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));
	}

	void render(const glm::mat4& projection, const glm::mat4& view)
	{
		// Render two cubes
		for (unsigned int i = 0; i < instanceCount; i++)
		{
			// Scale to 20cm: 200cm * 0.1
			cube->toWorld = instance_positions[i] * glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
			cube->draw(shaderID, projection, view);
		}

		// Render Skybox : remove view translation
		skybox->draw(shaderID, projection, view);
	}
};