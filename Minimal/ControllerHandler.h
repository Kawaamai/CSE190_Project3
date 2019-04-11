#pragma once

#ifndef CONTROLLER_HANDLER_H
#define CONTROLLER_HANDLER_H

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>

#include "Model.h"

class ControllerHandler
{
public:
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
	
	// hand model information
	std::string modelPath = "sphere2.obj";
	Model handPointer = Model(modelPath);
	Shader shader = Shader("basicVertex.vs", "basicFragment.fs");
	const glm::vec3 scale = glm::vec3(0.05f);

	ControllerHandler(const ovrSession & s);
	~ControllerHandler();

	void renderHands(const glm::mat4 & projection, const glm::mat4 & modelview);

private:

	void updateHands();
	void renderHand(const glm::mat4 & projection, const glm::mat4 & modelview, const ovrVector3f & handPosition);
};

#endif // !CONTROLLER_HANDLER_H
