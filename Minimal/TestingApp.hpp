#pragma once

#include "Core.h"
#include "OvrHelper.h"
#include "RiftApp.h"
#include "ControllerHandler.h"
#include "CaveScene.hpp"
#include "TestCubeSceneCave.hpp"
#include "Lighting.h"
#include "RingBuffer.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

// An example application that renders a simple cube
class ExampleApp : public RiftApp
{
	std::unique_ptr<CaveScene> scene;
	std::unique_ptr<TestCubeSceneCave> caveScene;
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
		caveScene = std::make_unique<TestCubeSceneCave>();
		controllers = std::make_unique<ControllerHandler>(_session, sceneLight);
		scene = std::make_unique<CaveScene>();
		// give the scene the function to use to render the cave scene
		//scene->renderCave = caveScene->render;
	}

	void shutdownGl() override
	{
		caveScene.reset();
	}

	void renderScene(const glm::mat4& projection, const glm::mat4& headPose) override
	{
		//caveScene->render(projection, glm::inverse(headPose));
	}

	// note: glm::inverse(headPose) is the camera matrix
	void renderScene(const glm::mat4& projection, const glm::mat4& headPose, ovrEyeType eye) override
	{
		// TODO: setup offscreen buffers
		// render screen to off screen buffers (done)
		//caveScene->render(projection, glm::inverse(headPose), eye);
		//scene->render(projection, glm::inverse(headPose), eye);
		// thank you lambda functions <3
		scene->render([&](const glm::mat4& projection, const glm::mat4& view, const ovrEyeType eye) {
			caveScene->render(projection, view, eye);
		}, projection, glm::inverse(headPose), eye);

		// render cubes with these textures
		controllers->renderHands(projection, glm::inverse(headPose));
	}

	void update() override {
		controllers->updateHandState();
	}

	void setBlackScreen() {
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	}

	void setNormClearColor() {
		glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
	}

	void handleInput() override {
		// cube scaling
		caveScene->updateTime();
		if (controllers->isThumbstickButtonDown(ovrHand_Left)) {
			std::cerr << "reset cube scale" << std::endl;
			caveScene->resetCubeScale();
		}
		if (controllers->isThumbstickLeft(ovrHand_Left)) {
			std::cerr << "decrease cube scale" << std::endl;
			caveScene->decreaseCubeScale();
		}
		if (controllers->isThumbstickRight(ovrHand_Left)) {
			std::cerr << "increase cube scale" << std::endl;
			caveScene->increaseCubeScale();
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