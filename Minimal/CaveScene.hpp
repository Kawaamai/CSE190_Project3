#pragma once

#include "Core.h"
#include "OvrHelper.h"

#include "BasicShader.h"
#include "Skybox.h"
#include "Plane.h"
#include "TexturedPlane.h"

#include <functional>

#define TEX_WIDTH 1080
#define TEX_HEIGHT 1080

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
	const float scale = 2.4f;
	std::array<glm::mat4, 3> instance_positions = {
		glm::rotate(glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::translate(glm::vec3(0.0f, 0.0f, -1.2f)) * glm::scale(glm::vec3(scale)),
		glm::rotate(glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::translate(glm::vec3(0.0f, 0.0f, -1.2f)) * glm::scale(glm::vec3(scale)),
		glm::rotate(glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::translate(glm::vec3(0.0f, 0.0f, -1.2f)) * glm::scale(glm::vec3(scale))
	};
	//std::unique_ptr<Plane> plane;
	std::unique_ptr<TexturedPlane> plane;

	GLuint m_fbo;
	GLuint m_texture;
	GLuint m_rbo;

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

		//plane = std::make_unique<Plane>();
		plane = std::make_unique<TexturedPlane>();

		// -------------------------------------------------------------------------------------------------------------------------------------------
		// TODO: setup offscreen buffeers
		// generate framebuffer
		glGenFramebuffers(1, &m_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

		// generate texture to render to
		glGenTextures(1, &m_texture);
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEX_WIDTH, TEX_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		// depth buffer
		glGenRenderbuffers(1, &m_rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, TEX_WIDTH, TEX_HEIGHT);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
		
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cerr << "ERROR WITH THE FRAME BUFFER SDKLJF:LSDKJF:SLDKFJ:SLKDFJ:SLKDJF:LSKJF:LSKJDF:LSKJF:LSKDJF:LSDJF:LSKFJ" << std::endl;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void render(
		const std::function<void(const glm::mat4&, const glm::mat4&, const ovrEyeType)> renderCave,
		const glm::mat4& projection,
		const glm::mat4& view,
		const ovrEyeType eye
	) {
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		glClearColor(0.1f, 0.1f, 1.0f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT);

		renderCave(projection, view, eye);
		// implicit call to glBindFramebuffer to eye bufferfs in passed in lambda function

		 //TODO: render the cave walls
		for (auto & ip : instance_positions) {
			plane->toWorld = ip;
			//plane->draw(basicShaderId, projection, view);
			plane->draw(projection, view, m_texture);
		}
		//plane->toWorld = instance_positions[0];
		//plane->draw(projection, view, m_texture);
		//renderCave(projection, view, eye);

		if (eye == ovrEye_Left)
			skybox->draw(shaderId, projection, view);
		else
			skyboxRight->draw(shaderId, projection, view);
	}
};