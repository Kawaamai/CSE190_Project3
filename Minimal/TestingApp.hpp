#pragma once

#include "Core.h"
#include "OvrHelper.h"
#include "RiftApp.h"
#include "ControllerHandler.h"
#include "CaveScene.hpp"
#include "Lighting.h"
#include "RingBuffer.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

// An example application that renders a simple cube
class ExampleApp : public RiftApp
{
	std::shared_ptr<CaveScene> scene;
	std::unique_ptr<ControllerHandler> controllers;

	// lighting for phong shading
	Lighting sceneLight = Lighting(glm::vec3(1.2f, 1.0f, 2.0f), glm::vec3(1.0f));

public:
	ExampleApp()
	{
	}

protected:
	void initGl() override
	{
		RiftApp::initGl();
		setBlackScreen();
		glEnable(GL_DEPTH_TEST);
		ovr_RecenterTrackingOrigin(_session);
		scene = std::shared_ptr<CaveScene>(new CaveScene());
		controllers = std::make_unique<ControllerHandler>(_session, sceneLight);
	}

	void shutdownGl() override
	{
		scene.reset();
	}

	void renderScene(const glm::mat4& projection, const glm::mat4& headPose) override
	{
		scene->render(projection, glm::inverse(headPose));
	}

	// note: glm::inverse(headPose) is the camera matrix
	void renderScene(const glm::mat4& projection, const glm::mat4& headPose, ovrEyeType eye) override
	{
		scene->render(projection, glm::inverse(headPose), eye);
		controllers->renderHands(projection, glm::inverse(headPose));
	}

	void update() override {
		controllers->updateHandState();
		//scene->updateTime();
	}

	void setBlackScreen() {
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	}

	void setNormClearColor() {
		glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
	}

	void handleInput() override {
		// cube scaling
		scene->updateTime();
		if (controllers->isThumbstickButtonDown(ovrHand_Left)) {
			std::cerr << "reset cube scale" << std::endl;
			scene->resetCubeScale();
		}
		if (controllers->isThumbstickLeft(ovrHand_Left)) {
			std::cerr << "decrease cube scale" << std::endl;
			scene->decreaseCubeScale();
		}
		if (controllers->isThumbstickRight(ovrHand_Left)) {
			std::cerr << "increase cube scale" << std::endl;
			scene->increaseCubeScale();
		}

		if (controllers->r_AButtonDown()) { }

		if (controllers->l_XButtonDown()) { }

		if (controllers->r_BButtonDown()) { }

		if (controllers->isThumbstickButtonDown(ovrHand_Right)) { }
		if (controllers->isThumbstickLeft(ovrHand_Right)) { }
		if (controllers->isThumbstickRight(ovrHand_Right)) { }

		if (controllers->r_IndexTriggerDown()) { }
		if (controllers->l_IndexTriggerDown()) { }

		if (controllers->r_HandTriggerDown()) { }
		if (controllers->l_HandTriggerDown()) { }
		if (controllers->l_YButtonDown()) { }

		if (controllers->isTouchThumbRestPressed(ovrHand_Left) && controllers->isTouchThumbRestPressed(ovrHand_Right)) { }
	}
};