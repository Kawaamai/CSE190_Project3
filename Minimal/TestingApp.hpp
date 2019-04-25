#pragma once

#include "Core.h"
#include "OvrHelper.h"
#include "RiftApp.h"
#include "ControllerHandler.h"
#include "TestCubeScene.hpp"
#include "Lighting.h"

// An example application that renders a simple cube
class ExampleApp : public RiftApp
{
	std::shared_ptr<TestCubeScene> scene;
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
		//glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
		setBlackScreen();
		glEnable(GL_DEPTH_TEST);
		ovr_RecenterTrackingOrigin(_session);
		scene = std::shared_ptr<TestCubeScene>(new TestCubeScene());
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

	void renderScene(const glm::mat4& projection, const glm::mat4& headPose, ovrEyeType eye) override
	{
		//update();
		//handleInput();
		scene->render(projection, glm::inverse(headPose), eye);
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
		if (controllers->isThumbstickButtonPressed(ovrHand_Left)) {
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

		// target eye rendering
		if (controllers->r_AButtonDown()) {
			std::cerr << "switch render setting" << std::endl;
			curEyeRenderState = eyeRenderMap.at(curEyeRenderState);
		}
	}
};