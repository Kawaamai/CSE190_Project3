#include "ControllerHandler.h"

ControllerHandler::ControllerHandler(const ovrSession & s) :
	_session(s)
{
}


ControllerHandler::~ControllerHandler()
{
}

void ControllerHandler::renderHands(const glm::mat4 & projection, const glm::mat4 & modelview)
{
	// update
	updateHands();

	// render hands
	if (handStatus[0]) {
		renderHand(projection, modelview, handPoses[0].Position);
	}
	if (handStatus[1]) {
		renderHand(projection, modelview, handPoses[1].Position);
	}
}

void ControllerHandler::updateHands()
{
	double displayMidpointSeconds = ovr_GetPredictedDisplayTime(_session, 0);
	ovrTrackingState trackState = ovr_GetTrackingState(_session, displayMidpointSeconds, ovrTrue);

	handStatus[0] = trackState.HandStatusFlags[0];
	handStatus[1] = trackState.HandStatusFlags[1];

	// Display status for debug purposes:
	//std::cerr << "handStatus[left]  = " << handStatus[ovrHand_Left] << std::endl;
	//std::cerr << "handStatus[right] = " << handStatus[ovrHand_Right] << std::endl;

	handPoses[0] = trackState.HandPoses[0].ThePose;
	handPoses[1] = trackState.HandPoses[1].ThePose;
	//ovrVector3f handPosition[2];
	//handPosition[0] = handPoses[0].Position;
	//handPosition[1] = handPoses[1].Position;
}

void ControllerHandler::renderHand(
	const glm::mat4 & projection,
	const glm::mat4 & modelview,
	const ovrVector3f & handPosition)
{
	glm::mat4 transform = glm::translate(glm::mat4(1.0), glm::vec3(handPosition.x, handPosition.y, handPosition.z));

	shader.use();
	shader.setMat4("ProjectionMatrix", projection);
	shader.setMat4("CameraMatrix", modelview);
	shader.setMat4("InstanceTransform", transform);
	shader.setMat4("ModelMatrix", glm::scale(glm::mat4(1.0), scale));
	handPointer.Draw(shader);
}
