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
	
	// Project 3

	// view perspective
	bool controllerView = false;
	bool updateScreen = true;

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
		//scene->render(projection, glm::inverse(headPose), eye);
		// thank you lambda functions <3
		scene->render([&](const glm::mat4& projection, const glm::mat4& view, const ovrEyeType eye) {
			//caveScene->render(projection, glm::scale(glm::vec3(1/2.4f)) * view, eye);
			caveScene->render(projection, view * glm::scale(glm::vec3(1/2.4f)), eye);
			//caveScene->render(projection, view, eye);
			returnToFbo();
		}, projection, glm::inverse(headPose), eye);

		controllers->renderHands(projection, glm::inverse(headPose));
	}

	/// with added eyePose
	void renderScene(const glm::mat4& projection, const glm::mat4& headPose, ovrEyeType eye, ovrPosef eyePose) override
	{
		if (controllerView) {
			ovrPosef eyePoseMod = controllers->handPoses[ovrHand_Right];
			//glm::vec3 eyePos = ovr::toGlm(eyePoseMod.Position) + glm::vec3(glm::mat4_cast(ovr::toGlm(controllers->handPoses[ovrHand_Right].Orientation)) * glm::vec4(ovr::toGlm(_viewScaleDesc.HmdToEyePose[eye].Position), 1.0f));
			//std::cerr << eye << "\t" << eyePos.x << " " << eyePos.y << " " << eyePos.z << std::endl;
			eyePoseMod.Position
				= ovr::fromGlm(ovr::toGlm(eyePoseMod.Position)
					+ glm::vec3(
						glm::mat4_cast(ovr::toGlm(controllers->handPoses[ovrHand_Right].Orientation))
						* glm::vec4(ovr::toGlm(_viewScaleDesc.HmdToEyePose[eye].Position), 1.0f)
					)
				);
			//caveScene->render(projection, glm::inverse(ovr::toGlm(eyePoseMod)), eye);
			eyePose = eyePoseMod;
		}

		//scene->render(projection, glm::inverse(headPose), eye);
		//caveScene->render(projection, glm::inverse(headPose), eye);
		// thank you lambda functions <3

		scene->render([&](const glm::mat4& p, const glm::mat4& v, const ovrEyeType e) {
			//caveScene->render(p, glm::scale(glm::vec3(1/2.4f)) * v, e);
			//caveScene->render(p, v * glm::scale(glm::vec3(1/2.4f)), e);
			caveScene->render(p, v, e);
			//controllers->renderHands(p, v);
			returnToFbo();
		}, [&]() {
			returnToFbo();
		}, projection, glm::inverse(headPose), eye, eyePose, updateScreen, controllerView);

		controllers->renderHands(projection, glm::inverse(headPose));
	}
	void prerenderScene(const glm::mat4& projection, const glm::mat4& headPose, ovrEyeType eye, ovrPosef eyePose) override
	{
		if (controllerView) {
			ovrPosef eyePoseMod = controllers->handPoses[ovrHand_Right];
			//glm::vec3 eyePos = ovr::toGlm(eyePoseMod.Position) + glm::vec3(glm::mat4_cast(ovr::toGlm(controllers->handPoses[ovrHand_Right].Orientation)) * glm::vec4(ovr::toGlm(_viewScaleDesc.HmdToEyePose[eye].Position), 1.0f));
			//std::cerr << eye << "\t" << eyePos.x << " " << eyePos.y << " " << eyePos.z << std::endl;
			eyePoseMod.Position
				= ovr::fromGlm(ovr::toGlm(eyePoseMod.Position)
					+ glm::vec3(
						glm::mat4_cast(ovr::toGlm(controllers->handPoses[ovrHand_Right].Orientation))
						* glm::vec4(ovr::toGlm(_viewScaleDesc.HmdToEyePose[eye].Position), 1.0f)
					)
				);
			//caveScene->render(projection, glm::inverse(ovr::toGlm(eyePoseMod)), eye);
			eyePose = eyePoseMod;
		}

		//scene->render(projection, glm::inverse(headPose), eye);
		//caveScene->render(projection, glm::inverse(headPose), eye);
		// thank you lambda functions <3

		scene->prerender([&](const glm::mat4& p, const glm::mat4& v, const ovrEyeType e) {
			//caveScene->render(p, glm::scale(glm::vec3(1/2.4f)) * v, e);
			//caveScene->render(p, v * glm::scale(glm::vec3(1/2.4f)), e);
			caveScene->render(p, v, e);
			//controllers->renderHands(p, v);
			returnToFbo();
		}, [&]() {
			returnToFbo();
		}, projection, glm::inverse(headPose), eye, eyePose, updateScreen, controllerView);

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

		// cube translation
		if (controllers->isThumbstickButtonDown(ovrHand_Right)) {
			caveScene->move(NONE);
		}
		if (controllers->isThumbstickUp(ovrHand_Left)) {
			caveScene->move(UP);
		}
		if (controllers->isThumbstickDown(ovrHand_Left)) {
			caveScene->move(DOWN);
		}
		if (controllers->isThumbstickUp(ovrHand_Right)) {
			caveScene->move(BACKWARD);
		}
		if (controllers->isThumbstickDown(ovrHand_Right)) {
			caveScene->move(FORWARD);
		}
		if (controllers->isThumbstickLeft(ovrHand_Right)) {
			caveScene->move(LEFT);
		}
		if (controllers->isThumbstickRight(ovrHand_Right)) {
			caveScene->move(RIGHT);
		}

		if (controllers->r_AButtonDown()) { }
		if (controllers->r_AButtonPressed()) {
			scene->debugLines = true;
		}
		else {
			scene->debugLines = false;
		}

		if (controllers->l_XButtonDown()) { }

		if (controllers->r_BButtonDown()) {
			updateScreen = !updateScreen;
		}

		if (controllers->isThumbstickButtonDown(ovrHand_Right)) { }
		if (controllers->isThumbstickLeft(ovrHand_Right)) { }
		if (controllers->isThumbstickRight(ovrHand_Right)) { }

		if (controllers->r_IndexTriggerDown()) { }
		if (controllers->r_IndexTriggerPressed()) {
			controllerView = true;
		}
		else {
			controllerView = false;
		}
		if (controllers->l_IndexTriggerDown()) { }

		if (controllers->r_HandTriggerDown()) { }
		if (controllers->l_HandTriggerDown()) { }
		if (controllers->l_YButtonDown()) {
			scene->toggleLightingMode();
		}

		if (controllers->isTouchThumbRestPressed(ovrHand_Left) && controllers->isTouchThumbRestPressed(ovrHand_Right)) { }
	}
};