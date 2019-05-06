#pragma once

#include "Core.h"
#include "OvrHelper.h"

#include "BasicShader.h"
#include "Skybox.h"

#include <functional>

class CaveScene
{
	GLuint shaderId;

	std::unique_ptr<Skybox> skybox;
	std::unique_ptr<Skybox> skyboxRight;

public:
	//void (*renderCave)(const glm::mat4&, const glm::mat4&, const ovrEyeType) = nullptr;
	//std::function (*renderCave)(const glm::mat4&, const glm::mat4&, const ovrEyeType);

	CaveScene() {
		// Shader Program 
		shaderId = LoadShaders("skybox.vert", "skybox.frag");

		// 10m wide sky box: size doesn't matter though
		skybox = std::make_unique<Skybox>("skybox");
		skybox->toWorld = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));
		skyboxRight = std::make_unique<Skybox>("skybox_righteye");
		skyboxRight->toWorld = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));
	}

	void render(
		const std::function<void(const glm::mat4&, const glm::mat4&, const ovrEyeType)> renderCave,
		const glm::mat4& projection,
		const glm::mat4& view,
		const ovrEyeType eye
	) {
		// TODO: setup and render to offscreen buffeers

		renderCave(projection, view, eye);

		// TODO: render the cave walls

		if (eye == ovrEye_Left)
			skybox->draw(shaderId, projection, view);
		else
			skyboxRight->draw(shaderId, projection, view);
	}
};