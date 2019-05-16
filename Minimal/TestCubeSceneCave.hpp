#pragma once

#include "Core.h"
#include "OvrHelper.h"

#include "BasicShader.h"
#include "Skybox.h"
#include <vector>

enum DIRECTION {
	UP, DOWN, LEFT, RIGHT, FORWARD, BACKWARD, NONE
};

// a class for building and rendering cubes
class TestCubeSceneCave
{
	// Program
	std::vector<glm::mat4> instance_positions;
	GLuint instanceCount;
	GLuint shaderID;

	std::unique_ptr<TexturedCube> cube;
	std::unique_ptr<Skybox> skybox;
	std::unique_ptr<Skybox> skyboxRight;

	const float initCubeScale = 0.1f; // 20cm = 200 * .1
	const float maxCubeScale = 0.25f; // 50cm = 200 * .25
	const float minCubeScale = 0.05f; // 10cm = 200 * .05
	const float cubeScaleInc = 0.05f;
	const float cubeTranslationInc = 0.20f;
	float curCubeScale = initCubeScale;
	glm::vec3 curCubeTranslation = glm::vec3(0.0f);

	// frame timing
	float currentTime;
	float lastTime;

public:

	TestCubeSceneCave()
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
		skyboxRight = std::make_unique<Skybox>("skybox_righteye");
		skyboxRight->toWorld = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));
		lastTime = ovr_GetTimeInSeconds();
	}

	//void render(const glm::mat4& projection, const glm::mat4& view)
	//{
	//	// Render two cubes
	//	for (unsigned int i = 0; i < instanceCount; i++)
	//	{
	//		// Scale to 20cm: 200cm * 0.1
	//		cube->toWorld = instance_positions[i] * glm::scale(glm::mat4(1.0f), glm::vec3(curCubeScale));
	//		cube->draw(shaderID, projection, view);
	//	}

	//	// Render Skybox : remove view translation
	//	skybox->draw(shaderID, projection, view);
	//}

	void render(const glm::mat4& projection, const glm::mat4& view, const ovrEyeType eye)
	{
		// Render two cubes
		for (unsigned int i = 0; i < instanceCount; i++)
		{
			// Scale to 20cm: 200cm * 0.1
			cube->toWorld = glm::translate(curCubeTranslation) * instance_positions[i] * glm::scale(glm::mat4(1.0f), glm::vec3(curCubeScale));
			cube->draw(shaderID, projection, view);
		}

		// Render Skybox : remove view translation
		if (eye == ovrEye_Left)
			skybox->draw(shaderID, projection, view);
		else
			skyboxRight->draw(shaderID, projection, view);
	}

	void increaseCubeScale() {
		curCubeScale += cubeScaleInc * getTimeDelta();
		if (curCubeScale > maxCubeScale)
			curCubeScale = maxCubeScale;
	}

	void decreaseCubeScale() {
		curCubeScale -= cubeScaleInc * getTimeDelta();
		if (curCubeScale < minCubeScale)
			curCubeScale = minCubeScale;
	}

	void move(DIRECTION dir) {
		switch (dir) {
		case RIGHT:
			curCubeTranslation += glm::vec3(cubeTranslationInc * getTimeDelta(), 0.0f, 0.0f);
			break;
		case LEFT:
			curCubeTranslation += glm::vec3(-cubeTranslationInc * getTimeDelta(), 0.0f, 0.0f);
			break;
		case FORWARD:
			curCubeTranslation += glm::vec3(0.0f, 0.0f, cubeTranslationInc * getTimeDelta());
			break;
		case BACKWARD:
			curCubeTranslation += glm::vec3(0.0f, 0.0f, -cubeTranslationInc * getTimeDelta());
			break;
		case UP:
			curCubeTranslation += glm::vec3(0.0f, cubeTranslationInc * getTimeDelta(), 0.0f);
			break;
		case DOWN:
			curCubeTranslation += glm::vec3(0.0f, -cubeTranslationInc * getTimeDelta(), 0.0f);
			break;
		default:
			curCubeTranslation = glm::vec3(0.0f);
		}
	}

	void resetCubeScale() {
		curCubeScale = initCubeScale;
	}

	float getTimeDelta() {
		return 1.0f;
		//return currentTime - lastTime;
	}

	void updateTime() {
		lastTime = currentTime;
		currentTime = ovr_GetTimeInSeconds();
	}
};