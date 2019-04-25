#pragma once

#include "Core.h"
#include "OvrHelper.h"
#include "RiftApp.h"
#include "TestCubeScene.hpp"

// An example application that renders a simple cube
class ExampleApp : public RiftApp
{
	std::shared_ptr<TestCubeScene> scene;

public:
	ExampleApp()
	{
	}

protected:
	void initGl() override
	{
		RiftApp::initGl();
		glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
		glEnable(GL_DEPTH_TEST);
		ovr_RecenterTrackingOrigin(_session);
		scene = std::shared_ptr<TestCubeScene>(new TestCubeScene());
	}

	void shutdownGl() override
	{
		scene.reset();
	}

	void renderScene(const glm::mat4& projection, const glm::mat4& headPose) override
	{
		scene->render(projection, glm::inverse(headPose));
	}
};