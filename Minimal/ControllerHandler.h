#pragma once

#ifndef CONTROLLER_HANDLER_H
#define CONTROLLER_HANDLER_H

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>
#include <GL/glew.h>

#pragma warning( disable : 4068 4244 4267 4065)
#include <oglplus/config/basic.hpp>
#include <oglplus/config/gl.hpp>
#include <oglplus/all.hpp>
#include <oglplus/interop/glm.hpp>
#include <oglplus/bound/texture.hpp>
#include <oglplus/bound/framebuffer.hpp>
#include <oglplus/bound/renderbuffer.hpp>
#include <oglplus/bound/buffer.hpp>
#include <oglplus/shapes/cube.hpp>
#include <oglplus/shapes/sphere.hpp>
#include <oglplus/shapes/wrapper.hpp>
#pragma warning( default : 4068 4244 4267 4065)

//#include "Model.h"
#include "Shader.h" // has glm.hpp
#include <glm/gtc/matrix_transform.hpp>

class ControllerHandler
{
public:
	const bool debug = false;

	ovrSession _session;

	// Process controller status. Useful to know if controller is being used at all, and if the cameras can see it. 
	// Bits reported:
	// Bit 1: ovrStatus_OrientationTracked  = Orientation is currently tracked (connected and in use)
	// Bit 2: ovrStatus_PositionTracked     = Position is currently tracked (false if out of range)
	unsigned int handStatus[2];
	// Process controller position and orientation:
	ovrPosef handPoses[2];  // These are position and orientation in meters in room coordinates, relative to tracking origin. Right-handed cartesian coordinates.
							// ovrQuatf     Orientation;
							// ovrVector3f  Position;
	ovrInputState prevInputState, currInputState;
	
	// hand model information
	//std::string modelPath = "sphere2.obj";
	const char *vertexShader = "oglBasicColor.vert";
	const char *fragShader = "oglBasicColor.frag";
	oglplus::shapes::ShapeWrapper sphere;
	const double baseDetectionRadius = 0.05;
	oglplus::Program prog;
	oglplus::VertexArray vao;
	GLuint instanceCount;
	oglplus::Buffer instances;
	oglplus::Buffer colors;
	// hand colors
	std::vector<glm::vec4> instance_colors = {
		glm::vec4(0.7f, 0, 0.7f, 1.0f),
		glm::vec4(0.7f, 0, 0.7f, 1.0f)
	};

	//Model handPointer = Model(modelPath);
	//Shader shader = Shader("basicVertex.vs", "basicFragment.fs");
	const glm::vec3 scale = glm::vec3(0.05f);

	ControllerHandler(const ovrSession & s);
	~ControllerHandler();

	void renderHands(const glm::mat4 & projection, const glm::mat4 & modelview);
	void updateHandState();

	// hand positions
	glm::vec3 getHandPosition(unsigned int hand) {
		ovrVector3f handPosition = handPoses[hand].Position;
		return glm::vec3(handPosition.x, handPosition.y, handPosition.z);
	}

	// button handlers
	bool r_HandTriggerDown() {
		return ((currInputState.HandTrigger[ovrHand_Right] > 0.5f) &&
			(prevInputState.HandTrigger[ovrHand_Right] < 0.5f));
	}
	bool l_HandTriggerDown() {
		return ((currInputState.HandTrigger[ovrHand_Left] > 0.5f) &&
			(prevInputState.HandTrigger[ovrHand_Left] < 0.5f));
	}
	bool r_IndexTriggerDown() {
		return ((currInputState.IndexTrigger[ovrHand_Right] > 0.5f) &&
			(prevInputState.IndexTrigger[ovrHand_Right] < 0.5f));
	}
	bool l_IndexTriggerDown() {
		return ((currInputState.IndexTrigger[ovrHand_Left] > 0.5f) &&
			(prevInputState.IndexTrigger[ovrHand_Left] < 0.5f));
	}
	bool r_AButtonDown() {
		return ((currInputState.Buttons & ovrButton_A) &&
			((prevInputState.Buttons & ovrButton_A) == 0));
	}
	bool r_BButtonDown() {
		return ((currInputState.Buttons & ovrButton_B) &&
			((prevInputState.Buttons & ovrButton_B) == 0));
	}
	bool l_XButtonDown() {
		return ((currInputState.Buttons & ovrButton_X) &&
			((prevInputState.Buttons & ovrButton_X) == 0));
	}
	bool l_YButtonDown() {
		return ((currInputState.Buttons & ovrButton_Y) &&
			((prevInputState.Buttons & ovrButton_Y) == 0));
	}

private:

	void updateHands();
	void buttonHandler();
	//void renderHand(const glm::mat4 & projection, const glm::mat4 & modelview, const ovrVector3f & handPosition);
};

#endif // !CONTROLLER_HANDLER_H
