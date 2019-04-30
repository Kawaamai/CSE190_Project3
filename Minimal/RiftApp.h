#pragma once

#include <algorithm>
#include <vector>
#include <array>
#include "Core.h"
#include "GlfwApp.h"
#include "AvatarHandler.h"
#include "RingBuffer.h"


class RiftManagerApp {
protected:
	ovrSession _session;
	ovrHmdDesc _hmdDesc;
	ovrGraphicsLuid _luid;

public:
	RiftManagerApp() {
		if (!OVR_SUCCESS(ovr_Create(&_session, &_luid))) {
			FAIL("Unable to create HMD session");
		}

		_hmdDesc = ovr_GetHmdDesc(_session);
	}

	~RiftManagerApp() {
		ovr_Destroy(_session);
		_session = nullptr;
	}
};

class RiftApp : public GlfwApp, public RiftManagerApp {
public:

private:
	GLuint _fbo{ 0 };
	GLuint _depthBuffer{ 0 };
	ovrTextureSwapChain _eyeTexture;

	GLuint _mirrorFbo{ 0 };
	ovrMirrorTexture _mirrorTexture;

	ovrEyeRenderDesc _eyeRenderDescs[2];

	mat4 _eyeProjections[2];

	ovrLayerEyeFov _sceneLayer;
	ovrViewScaleDesc _viewScaleDesc;

	// TODO: delete
	ovrViewScaleDesc _viewScaleDescBase;
	const float minIPD = -.1f / 2; // based on right eye
	const float maxIPD = .3f / 2; // based on right eye

	// head tracking lag
	// this will run into problems later when when doing the mono and other stuff
	std::array<std::array<glm::mat4, 2>, 30> lagBuffer;
	int lagIdx = 0;
	int lag = 0;
	int delay = 0;
	int currentDelay = 0;

	uvec2 _renderTargetSize;
	uvec2 _mirrorSize;

	std::unique_ptr<AvatarHandler> av;

public:

	RiftApp() {
		using namespace ovr;
		_viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;

		memset(&_sceneLayer, 0, sizeof(ovrLayerEyeFov));
		_sceneLayer.Header.Type = ovrLayerType_EyeFov;
		_sceneLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;

		ovr::for_each_eye([&](ovrEyeType eye) {
			ovrEyeRenderDesc& erd = _eyeRenderDescs[eye] = ovr_GetRenderDesc(_session, eye, _hmdDesc.DefaultEyeFov[eye]);
			ovrMatrix4f ovrPerspectiveProjection =
				ovrMatrix4f_Projection(erd.Fov, 0.01f, 1000.0f, ovrProjection_ClipRangeOpenGL);
			_eyeProjections[eye] = ovr::toGlm(ovrPerspectiveProjection);
			_viewScaleDesc.HmdToEyePose[eye] = erd.HmdToEyePose;

			ovrFovPort & fov = _sceneLayer.Fov[eye] = _eyeRenderDescs[eye].Fov;
			auto eyeSize = ovr_GetFovTextureSize(_session, eye, fov, 1.0f);
			_sceneLayer.Viewport[eye].Size = eyeSize;
			_sceneLayer.Viewport[eye].Pos = { (int)_renderTargetSize.x, 0 };

			_renderTargetSize.y = std::max(_renderTargetSize.y, (uint32_t)eyeSize.h);
			_renderTargetSize.x += eyeSize.w;
		});
		// Make the on screen window 1/4 the resolution of the render target
		_mirrorSize = _renderTargetSize;
		_mirrorSize /= 4;

		_viewScaleDescBase = _viewScaleDesc;
	}

protected:
	//----------- variables
	enum EYE_RENDER_STATE {
		BOTH, MONO, RIGHT, LEFT, SWITCHED
	};
	const std::map<EYE_RENDER_STATE, EYE_RENDER_STATE> eyeRenderMap {
		{BOTH, MONO},
		{MONO, RIGHT},
		{RIGHT, LEFT},
		{LEFT, SWITCHED},
		{SWITCHED, BOTH}
	};
	EYE_RENDER_STATE curEyeRenderState = BOTH;

	enum TRACKING_MODE {
		NORMAL, POSITION, ORIENTATION
	};

	std::map<TRACKING_MODE, TRACKING_MODE> trackingModeMap{
		{NORMAL, POSITION}, {POSITION, ORIENTATION}, {ORIENTATION, NORMAL}
	};

	TRACKING_MODE currentTrackingMode = NORMAL;
	std::array<ovrQuatf, 2> savedOrientation;
	std::array<ovrVector3f, 2> savedTranslation;

	GLFWwindow * createRenderingTarget(uvec2 & outSize, ivec2 & outPosition) override {
		return glfw::createWindow(_mirrorSize);
	}

	//------------ functions

	void initGl() override {
		GlfwApp::initGl();

		// Disable the v-sync for buffer swap
		glfwSwapInterval(0);

		ovrTextureSwapChainDesc desc = {};
		desc.Type = ovrTexture_2D;
		desc.ArraySize = 1;
		desc.Width = _renderTargetSize.x;
		desc.Height = _renderTargetSize.y;
		desc.MipLevels = 1;
		desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		desc.SampleCount = 1;
		desc.StaticImage = ovrFalse;
		ovrResult result = ovr_CreateTextureSwapChainGL(_session, &desc, &_eyeTexture);
		_sceneLayer.ColorTexture[0] = _eyeTexture;
		if (!OVR_SUCCESS(result)) {
			FAIL("Failed to create swap textures");
		}

		int length = 0;
		result = ovr_GetTextureSwapChainLength(_session, _eyeTexture, &length);
		if (!OVR_SUCCESS(result) || !length) {
			FAIL("Unable to count swap chain textures");
		}
		for (int i = 0; i < length; ++i) {
			GLuint chainTexId;
			ovr_GetTextureSwapChainBufferGL(_session, _eyeTexture, i, &chainTexId);
			glBindTexture(GL_TEXTURE_2D, chainTexId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		glBindTexture(GL_TEXTURE_2D, 0);

		// Set up the framebuffer object
		glGenFramebuffers(1, &_fbo);
		glGenRenderbuffers(1, &_depthBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
		glBindRenderbuffer(GL_RENDERBUFFER, _depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _renderTargetSize.x, _renderTargetSize.y);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		ovrMirrorTextureDesc mirrorDesc;
		memset(&mirrorDesc, 0, sizeof(mirrorDesc));
		mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		mirrorDesc.Width = _mirrorSize.x;
		mirrorDesc.Height = _mirrorSize.y;
		if (!OVR_SUCCESS(ovr_CreateMirrorTextureGL(_session, &mirrorDesc, &_mirrorTexture))) {
			FAIL("Could not create mirror texture");
		}
		glGenFramebuffers(1, &_mirrorFbo);

		// FIXME: find a better place for this
		av = std::make_unique<AvatarHandler>(_session);
	}

	void onKey(int key, int scancode, int action, int mods) override {
		if (GLFW_PRESS == action) switch (key) {
		case GLFW_KEY_R:
			ovr_RecenterTrackingOrigin(_session);
			return;
		}

		GlfwApp::onKey(key, scancode, action, mods);
	}

	void draw() final override {
		ovrPosef eyePoses[2];
		ovr_GetEyePoses(_session, frame, true, _viewScaleDesc.HmdToEyePose, eyePoses, &_sceneLayer.SensorSampleTime);

		handleInput();

		ovr::for_each_eye([&](ovrEyeType eye) {
			saveCameraBuffer(ovr::toGlm(eyePoses[eye]), eye);

			// save if neccessary
			if (currentTrackingMode != ORIENTATION)
				savedOrientation[eye] = eyePoses[eye].Orientation;
			if (currentTrackingMode != POSITION)
				savedTranslation[eye] = eyePoses[eye].Position;
		});

		bool render = true;
		if (delay) {
			if (currentDelay < delay) {
				render = false;
				currentDelay++;
			}
			else {
				currentDelay = 0;
			}
		}

		if (render) {
			int curIndex;
			ovr_GetTextureSwapChainCurrentIndex(_session, _eyeTexture, &curIndex);
			GLuint curTexId;
			ovr_GetTextureSwapChainBufferGL(_session, _eyeTexture, curIndex, &curTexId);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			ovr::for_each_eye([&](ovrEyeType eye) {
				if ((curEyeRenderState == RIGHT && eye == ovrEye_Left) ||
					(curEyeRenderState == LEFT && eye == ovrEye_Right)) {
					return;
				}

				const auto& vp = _sceneLayer.Viewport[eye];
				glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);

				_sceneLayer.RenderPose[eye] = eyePoses[eye]; // do before switch.

				if (curEyeRenderState == SWITCHED) {
					if (eye == ovrEye_Left)
						eye = ovrEye_Right;
					else
						eye = ovrEye_Left;
				}
				else if (curEyeRenderState == MONO && eye == ovrEye_Right) {
					eye = ovrEye_Left;
				}

				// replace with saved
				if (currentTrackingMode == ORIENTATION)
					eyePoses[eye].Orientation = savedOrientation[eye];
				if (currentTrackingMode == POSITION)
					eyePoses[eye].Position = savedTranslation[eye];

				// hand avatar rendering
				{
					ovrVector3f eyePosition = eyePoses[eye].Position;
					glm::vec3 eyeWorld = ovr::toGlm(eyePosition);
					if (!lag) {
						glm::mat4 view = glm::inverse(ovr::toGlm(eyePoses[eye]));
						av->updateAvatar(_eyeProjections[eye], view, eyeWorld);
					}
					else {
						glm::mat4 headPose = getRingAt(lagBuffer, lagIdx - lag)[eye];
						av->updateAvatar(_eyeProjections[eye], glm::inverse(headPose), eyeWorld);
					}
				}

				//renderScene(_eyeProjections[eye], ovr::toGlm(eyePoses[eye])); // score on hand
				if (!lag) {
					renderScene(_eyeProjections[eye], ovr::toGlm(eyePoses[eye]), eye); // score on hand
				}
				else {
					glm::mat4 headPose = getRingAt(lagBuffer, lagIdx - lag)[eye];
					renderScene(_eyeProjections[eye], headPose, eye); // score on hand
				}
				//renderScene(_eyeProjections[eye], ovr::toGlm(eyePoses[eye]), eyePoses[eye]); // score in hud
			});
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			ovr_CommitTextureSwapChain(_session, _eyeTexture);
			ovrLayerHeader* headerList = &_sceneLayer.Header;
			ovr_SubmitFrame(_session, frame, &_viewScaleDesc, &headerList, 1);
		}
		else {
			ovr::for_each_eye([&](ovrEyeType eye) {
				_sceneLayer.RenderPose[eye] = eyePoses[eye];
			});
			ovrLayerHeader* headerList = &_sceneLayer.Header;
			ovr_SubmitFrame(_session, frame, &_viewScaleDesc, &headerList, 1);
		}

		// mirrors oculus frame buffer to debug screen frame buffer
		GLuint mirrorTextureId;
		ovr_GetMirrorTextureBufferGL(_session, _mirrorTexture, &mirrorTextureId);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, _mirrorFbo);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTextureId, 0);
		glBlitFramebuffer(0, 0, _mirrorSize.x, _mirrorSize.y, 0, _mirrorSize.y, _mirrorSize.x, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

		std::cerr << "Tracking Lag: " << lag << " frames" << std::endl;
		std::cerr << "Rendering delay: " << (delay + 1) << " frames" << std::endl;
	}

	void lateUpdate() override {
		lagIdx = incRingIdx(lagBuffer, lagIdx);
	}

	void saveCameraBuffer(const glm::mat4& camera, ovrEyeType eye) {
		getRingAt(lagBuffer, lagIdx)[eye] = camera;
	}

	void incIPD() {
		_viewScaleDesc.HmdToEyePose[ovrEye_Left].Position.x -= .005f;
		_viewScaleDesc.HmdToEyePose[ovrEye_Right].Position.x += .005f;
		if (_viewScaleDesc.HmdToEyePose[ovrEye_Right].Position.x > maxIPD) {
			_viewScaleDesc.HmdToEyePose[ovrEye_Left].Position.x = -maxIPD;
			_viewScaleDesc.HmdToEyePose[ovrEye_Right].Position.x = maxIPD;
		}
	}

	void decIPD() {
		_viewScaleDesc.HmdToEyePose[ovrEye_Left].Position.x += .005f;
		_viewScaleDesc.HmdToEyePose[ovrEye_Right].Position.x -= .005f;
		if (_viewScaleDesc.HmdToEyePose[ovrEye_Right].Position.x < minIPD) {
			_viewScaleDesc.HmdToEyePose[ovrEye_Left].Position.x = -minIPD;
			_viewScaleDesc.HmdToEyePose[ovrEye_Right].Position.x = minIPD;
		}
	}

	void resetIPD() {
		_viewScaleDesc = _viewScaleDescBase;
	}

	void incLag() {
		lag++;
	}

	void decLag() {
		lag--;
		if (lag < 0)
			lag = 0;
	}

	void resetLag() {
		lag = 0;
	}

	int getlag() {
		return lag;
	}

	void incDelay() {
		delay++;
		if (delay > 10)
			delay = 91;
	}

	void decDelay() {
		if (delay == 90)
			delay = 10;
		delay--;
		if (delay < 0) {
			delay = 0;
			currentDelay = 0;
		}
	}

	//void update() {}
	virtual void handleInput() = 0;
	virtual void renderScene(const glm::mat4 & projection, const glm::mat4 & headPose) = 0;
	virtual void renderScene(const glm::mat4 & projection, const glm::mat4 & headPose, ovrEyeType eye) = 0;
	//virtual void renderScene(const glm::mat4 & projection, const glm::mat4 & headPose, const ovrPosef camera) = 0;
	//virtual void renderScene(const glm::mat4 & projection, const glm::mat4 & headPose, const glm::vec3 & headPos) = 0;
};