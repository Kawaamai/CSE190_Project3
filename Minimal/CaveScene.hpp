#pragma once

#include "Core.h"
#include "OvrHelper.h"

#include "BasicShader.h"
#include "Skybox.h"
#include "Plane.h"

#include <functional>

class CaveScene
{
	GLuint shaderId;
	GLuint basicShaderId;

	std::unique_ptr<Skybox> skybox;
	std::unique_ptr<Skybox> skyboxRight;

	// unscaled
	//std::array<glm::mat4, 3> instance_positions = {
	//	glm::translate(glm::vec3(0.0f, 0.0f, -0.5f)),
	//	glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::translate(glm::vec3(0.0f, 0.0f, -0.5f)),
	//	glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::translate(glm::vec3(0.0f, 0.0f, -0.5f))
	//};
	// scaled
	std::array<glm::mat4, 3> instance_positions = {
		glm::translate(glm::vec3(0.0f, 0.0f, -1.2f)),
		glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::translate(glm::vec3(0.0f, 0.0f, -1.2f)),
		glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::translate(glm::vec3(0.0f, 0.0f, -1.2f))
	};
	std::unique_ptr<Plane> plane;

public:
	//void (*renderCave)(const glm::mat4&, const glm::mat4&, const ovrEyeType) = nullptr;
	//std::function (*renderCave)(const glm::mat4&, const glm::mat4&, const ovrEyeType);

	CaveScene() {
		// Shader Program 
		shaderId = LoadShaders("skybox.vert", "skybox.frag");
		basicShaderId = LoadShaders("basicColor.vert", "basicColor.frag");

		// 10m wide sky box: size doesn't matter though
		skybox = std::make_unique<Skybox>("skybox");
		skybox->toWorld = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));
		skyboxRight = std::make_unique<Skybox>("skybox_righteye");
		skyboxRight->toWorld = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));

		plane = std::make_unique<Plane>();
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
		for (auto & ip : instance_positions) {
			plane->toWorld = glm::rotate(glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f))
				* ip
				* glm::scale(glm::vec3(2.4f));
			plane->draw(basicShaderId, projection, view);
		}
		//plane->toWorld = glm::translate(glm::vec3(0.0f, 0.0f, -0.5f));
		//plane->draw(basicShaderId, projection, view);
		//plane->toWorld = glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::translate(glm::vec3(0.0f, 0.0f, -0.5f));
		//plane->draw(basicShaderId, projection, view);
		//plane->toWorld = glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::translate(glm::vec3(0.0f, 0.0f, -0.5f));
		//plane->draw(basicShaderId, projection, view);

		if (eye == ovrEye_Left)
			skybox->draw(shaderId, projection, view);
		else
			skyboxRight->draw(shaderId, projection, view);
	}
};