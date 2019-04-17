/************************************************************************************

Authors     :   Bradley Austin Davis <bdavis@saintandreas.org>
Copyright   :   Copyright Brad Davis. All Rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************************/


#include <memory>
#include <exception>
#include <algorithm>

#include <Windows.h>

#define __STDC_FORMAT_MACROS 1
#define APP_ID "958062084316416"

///////////////////////////////////////////////////////////////////////////////
//
// GLM is a C++ math library meant to mirror the syntax of GLSL 
//

#include <glm/gtc/noise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

// Import the most commonly used types into the default namespace
using glm::ivec3;
using glm::ivec2;
using glm::uvec2;
using glm::mat3;
using glm::mat4;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::quat;

///////////////////////////////////////////////////////////////////////////////
//
// GLEW gives cross platform access to OpenGL 3.x+ functionality.  
//

#include "Core.h"

bool checkFramebufferStatus(GLenum target = GL_FRAMEBUFFER) {
	GLuint status = glCheckFramebufferStatus(target);
	switch (status) {
	case GL_FRAMEBUFFER_COMPLETE:
		return true;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		std::cerr << "framebuffer incomplete attachment" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		std::cerr << "framebuffer missing attachment" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		std::cerr << "framebuffer incomplete draw buffer" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		std::cerr << "framebuffer incomplete read buffer" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		std::cerr << "framebuffer incomplete multisample" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
		std::cerr << "framebuffer incomplete layer targets" << std::endl;
		break;

	case GL_FRAMEBUFFER_UNSUPPORTED:
		std::cerr << "framebuffer unsupported internal format or image" << std::endl;
		break;

	default:
		std::cerr << "other framebuffer error" << std::endl;
		break;
	}

	return false;
}

bool checkGlError() {
	GLenum error = glGetError();
	if (!error) {
		return false;
	}
	else {
		switch (error) {
		case GL_INVALID_ENUM:
			std::cerr << ": An unacceptable value is specified for an enumerated argument.The offending command is ignored and has no other side effect than to set the error flag.";
			break;
		case GL_INVALID_VALUE:
			std::cerr << ": A numeric argument is out of range.The offending command is ignored and has no other side effect than to set the error flag";
			break;
		case GL_INVALID_OPERATION:
			std::cerr << ": The specified operation is not allowed in the current state.The offending command is ignored and has no other side effect than to set the error flag..";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			std::cerr << ": The framebuffer object is not complete.The offending command is ignored and has no other side effect than to set the error flag.";
			break;
		case GL_OUT_OF_MEMORY:
			std::cerr << ": There is not enough memory left to execute the command.The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
			break;
		case GL_STACK_UNDERFLOW:
			std::cerr << ": An attempt has been made to perform an operation that would cause an internal stack to underflow.";
			break;
		case GL_STACK_OVERFLOW:
			std::cerr << ": An attempt has been made to perform an operation that would cause an internal stack to overflow.";
			break;
		}
		return true;
	}
}

void glDebugCallbackHandler(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, GLvoid* data) {
	OutputDebugStringA(msg);
	std::cout << "debug call: " << msg << std::endl;
}

//////////////////////////////////////////////////////////////////////
//
// GLFW provides cross platform window creation
//


#include "OvrHelper.h"
#include "RiftApp.h"


//////////////////////////////////////////////////////////////////////
//
// The remainder of this code is specific to the scene we want to 
// render.  I use oglplus to render an array of cubes, but your 
// application would perform whatever rendering you want
//


//////////////////////////////////////////////////////////////////////
//
// OGLplus is a set of wrapper classes for giving OpenGL a more object
// oriented interface
//
#define OGLPLUS_USE_GLCOREARB_H 0
#define OGLPLUS_USE_GLEW 1
#define OGLPLUS_USE_BOOST_CONFIG 0
#define OGLPLUS_NO_SITE_CONFIG 1
#define OGLPLUS_LOW_PROFILE 1

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

//static const char * VERTEX_SHADER = R"SHADER(
//#version 410 core
//
//uniform mat4 ProjectionMatrix = mat4(1);
//uniform mat4 CameraMatrix = mat4(1);
//
//layout(location = 0) in vec4 Position;
//layout(location = 2) in vec3 Normal;
//layout(location = 5) in mat4 InstanceTransform;
//
//out vec3 vertNormal;
//
//void main(void) {
//   mat4 ViewXfm = CameraMatrix * InstanceTransform;
//   //mat4 ViewXfm = CameraMatrix;
//   vertNormal = Normal;
//   gl_Position = ProjectionMatrix * ViewXfm * Position;
//}
//)SHADER";
//
//static const char * FRAGMENT_SHADER = R"SHADER(
//#version 410 core
//
//in vec3 vertNormal;
//out vec4 fragColor;
//
//void main(void) {
//    vec3 color = vertNormal;
//    if (!all(equal(color, abs(color)))) {
//        color = vec3(1.0) - abs(color);
//    }
//    fragColor = vec4(color, 1.0);
//}
//)SHADER";
//
//// a class for encapsulating building and rendering an RGB cube
//struct ColorCubeScene {
//
//	// Program
//	oglplus::shapes::ShapeWrapper cube;
//	oglplus::Program prog;
//	oglplus::VertexArray vao;
//	GLuint instanceCount;
//	oglplus::Buffer instances;
//
//	// VBOs for the cube's vertices and normals
//
//	const unsigned int GRID_SIZE{ 5 };
//
//public:
//	ColorCubeScene() : cube({ "Position", "Normal" }, oglplus::shapes::Cube()) {
//		using namespace oglplus;
//		try {
//			// attach the shaders to the program
//			prog.AttachShader(
//				FragmentShader()
//				.Source(GLSLSource(String(FRAGMENT_SHADER)))
//				.Compile()
//			);
//			prog.AttachShader(
//				VertexShader()
//				.Source(GLSLSource(String(VERTEX_SHADER)))
//				.Compile()
//			);
//			prog.Link();
//		}
//		catch (ProgramBuildError & err) {
//			FAIL((const char*)err.what());
//		}
//
//		// link and use it
//		prog.Use();
//
//		vao = cube.VAOForProgram(prog);
//		vao.Bind();
//		// Create a cube of cubes
//		{
//			std::vector<mat4> instance_positions;
//			for (unsigned int z = 0; z < GRID_SIZE; ++z) {
//				for (unsigned int y = 0; y < GRID_SIZE; ++y) {
//					for (unsigned int x = 0; x < GRID_SIZE; ++x) {
//						int xpos = (x - (GRID_SIZE / 2)) * 2;
//						int ypos = (y - (GRID_SIZE / 2)) * 2;
//						int zpos = (z - (GRID_SIZE / 2)) * 2;
//						vec3 relativePosition = vec3(xpos, ypos, zpos);
//						if (relativePosition == vec3(0)) {
//							continue;
//						}
//						instance_positions.push_back(glm::translate(glm::mat4(1.0f), relativePosition));
//					}
//				}
//			}
//
//			Context::Bound(Buffer::Target::Array, instances).Data(instance_positions);
//			instanceCount = (GLuint)instance_positions.size();
//			int stride = sizeof(mat4);
//			for (int i = 0; i < 4; ++i) {
//				VertexArrayAttrib instance_attr(prog, Attribute::InstanceTransform + i);
//				size_t offset = sizeof(vec4) * i;
//				instance_attr.Pointer(4, DataType::Float, false, stride, (void*)offset);
//				instance_attr.Divisor(1);
//				instance_attr.Enable();
//			}
//		}
//	}
//
//	void render(const mat4 & projection, const mat4 & modelview) {
//		using namespace oglplus;
//		prog.Use();
//		Uniform<mat4>(prog, "ProjectionMatrix").Set(projection);
//		Uniform<mat4>(prog, "CameraMatrix").Set(modelview);
//		vao.Bind();
//		cube.Draw(instanceCount);
//	}
//};

//#include "Model.h"
#include "SphereScene.h"
#include <chrono>

#include "AvatarHandler.h"
#include "Lighting.h"
#include "TextRenderer.h"

// An example application that renders a simple cube
class ExampleApp : public RiftApp {
	// hands
	std::unique_ptr<ControllerHandler> controllers;

	// sphere grid
	//std::shared_ptr<SphereScene> sphereScene;
	std::unique_ptr<SphereScene> sphereScene;

	//std::unique_ptr<AvatarHandler> av;

	// game
	bool gameStarted;
	unsigned int correctClicks;
	unsigned int totalClicks;
	const int maxGameTime = 60;
	std::chrono::steady_clock::time_point startTime;

	// object interaction
	bool grabbing;
	glm::vec3 grabOffset;

	Lighting sceneLight = Lighting(glm::vec3(1.2f, 1.0f, 2.0f), glm::vec3(1.0f));

	TextRenderer scoreText = TextRenderer("fonts/arial.ttf");

public:
	ExampleApp() :
		gameStarted(false), correctClicks(0), totalClicks(0), grabbing(false)
	{ }

protected:
	void initGl() override {
		RiftApp::initGl();
		//glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
		glClearColor(0.9f, 0.9f, 0.9f, 0.0f); // background color
		glEnable(GL_DEPTH_TEST);
		ovr_RecenterTrackingOrigin(_session);
		//sphereScene = std::shared_ptr<OglSphereScene>(new OglSphereScene());
		//controllers = std::shared_ptr<ControllerHandler>(new ControllerHandler(_session));
		// Note: to disable lighting, don't pass in sceneLight
		sphereScene = std::make_unique<SphereScene>(sceneLight);
		controllers = std::make_unique<ControllerHandler>(_session, sceneLight);
		//av = std::make_unique<AvatarHandler>(_session);
	}

	void shutdownGl() override {
		sphereScene.reset();
	}

	void renderScene(const glm::mat4 & projection, const glm::mat4 & headPose) override {
		controllers->updateHandState();
		handleInteractions();
		controllers->renderHands(projection, glm::inverse(headPose));
		if (gameStarted) {
			sphereScene->render(projection, glm::inverse(headPose));
			// TODO: show game overlay
		}

		//scoreText.renderText(projection, "asdfasdfasdfasdf", 25.0f, 25.0f, 1.0f, glm::vec3(0.5f, 0.8f, 0.2f));

		if (gameStarted && checkEndGameState()) {
			endGame();
		}
	}

	//void renderScene(const glm::mat4 & projection, const glm::mat4 & headPose, const glm::vec3 & headPos) override {
	//	controllers->updateHandState();
	//	handleInteractions();
	//	controllers->renderHands(projection, glm::inverse(headPose));
	//	if (gameStarted) {
	//		sphereScene->render(projection, glm::inverse(headPose));
	//		// TODO: show game overlay
	//	}

	//	//av->updateAvatar(projection, headPose, headPos);

	//	if (gameStarted && checkEndGameState()) {
	//		endGame();
	//	}
	//}

private:

	void handleInteractions() {
		float handToHighlightSphereDist = 10000.0f;

		if (gameStarted) {
			// calc distance
			handToHighlightSphereDist = glm::distance(
				//controllers->getHandPosition(ovrHand_Right),
				controllers->getPointerPosition(),
				glm::vec3(
					sphereScene->instance_positions[sphereScene->highlightedSphere] * glm::vec4(0, 0, 0, 1.0f)
				)
			);

			// grabbing
			if (grabbing) {
				if (controllers->l_HandTriggerUp()) {
					grabbing = false;
					std::cerr << "not grabbing" << std::endl;
				}
				else {
					glm::vec3 handPos = controllers->getHandPosition(ovrHand_Left);
					glm::mat4 handRotChange = glm::mat4_cast(controllers->getHandRotationChange(ovrHand_Left));
					glm::vec3 newOffset = handRotChange * glm::vec4(grabOffset, 1.0f);
					glm::vec3 currGridPos = sphereScene->toWorld * glm::vec4(0, 0, 0, 1);
					glm::mat4 additionalTrans = glm::translate(glm::mat4(), (handPos + grabOffset) - currGridPos);
					glm::vec3 handPosChange = controllers->getHandPositionChange(ovrHand_Left);
					glm::mat4 handPosChangeMat = glm::translate(glm::mat4(), handPosChange);
					// translation from hand rotation due to grab location offset
					sphereScene->toWorld
						= handPosChangeMat * additionalTrans * handRotChange * sphereScene->toWorld;
					grabOffset = newOffset;
				}
			}
			// TODO: there is a problem with the grabbing after we move it
			else { // is not grabbing
				//float handToSphereGridDist
				//	= glm::distance(controllers->getHandPosition(ovrHand_Left),
				//		glm::vec3(sphereScene->translation * sphereScene->orientation * glm::vec4(0.0f)));
				float handToSphereGridDist
					= glm::distance(controllers->getHandPosition(ovrHand_Left),
						glm::vec3(sphereScene->toWorld * glm::vec4(0, 0, 0, 1)));
				//std::cerr << handToSphereGridDist << std::endl;

				if (controllers->l_HandTriggerDown() &&
					handToSphereGridDist < 0.6f) { // rough sphere around the grid
					grabbing = true;

					grabOffset = glm::vec3(sphereScene->toWorld * glm::vec4(0, 0, 0, 1))
						- controllers->getHandPosition(ovrHand_Left);

					std::cerr << "grabbing" << std::endl;
				}
			}
		}

		if (controllers->r_IndexTriggerDown()) {
			if (!gameStarted) {
				startGame();
			}
			else {
				std::cerr << handToHighlightSphereDist << std::endl;
				if (handToHighlightSphereDist < (sphereScene->sphereRadius + controllers->baseDetectionRadius)) {
					correctClicks++;
					sphereScene->chooseNewHighlightSphere();
				}
				totalClicks++;
			}
		}
	}

	void startGame() {
		sphereScene->resetPositions();
		gameStarted = true;
		correctClicks = 0;
		totalClicks = 0;
		grabbing = false;
		std::cout << "Start Game" << std::endl;
		startTime = std::chrono::steady_clock::now();
	}
	
	bool checkEndGameState() {
		std::chrono::steady_clock::time_point t = std::chrono::steady_clock::now();
		auto time_span = std::chrono::duration_cast<std::chrono::seconds>(t - startTime).count();
		//std::cerr << "time: " << time_span << std::endl;
		return time_span >= maxGameTime;
	}

	void endGame() {
		gameStarted = false;
		grabbing = false;
		// TODO: show end game stuff
		std::cout << "End Game" << std::endl;
		std::cout << "correct number of clicked spheres: "
			+ std::to_string(correctClicks)
			+ "/" + std::to_string(totalClicks)
			<< std::endl;
	}
};

// Execute our example class
//int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
//	int result = -1;
//	try {
//		if (!OVR_SUCCESS(ovr_Initialize(nullptr))) {
//			FAIL("Failed to initialize the Oculus SDK");
//		}
//		result = ExampleApp().run();
//	}
//	catch (std::exception & error) {
//		OutputDebugStringA(error.what());
//		std::cerr << error.what() << std::endl;
//	}
//	ovr_Shutdown();
//	return result;
//}

// Execute our example class
int main(int argc, char **argv) {
	int result = -1;
	try {
		if (!OVR_SUCCESS(ovr_Initialize(nullptr))) {
			FAIL("Failed to initialize the Oculus SDK");
		}
		ovrAvatar_Initialize(APP_ID);

		result = ExampleApp().run();
	}
	catch (std::exception & error) {
		OutputDebugStringA(error.what());
		std::cerr << error.what() << std::endl;
	}
	//ovr_Shutdown();
	ovrAvatar_Shutdown();
	return result;
}