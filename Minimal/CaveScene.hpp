#pragma once

#include "Core.h"
#include "OvrHelper.h"

#include "BasicShader.h"
#include "Skybox.h"
#include "Plane.h"
#include "TexturedPlane.h"

#include <functional>

#define TEX_WIDTH 1080
#define TEX_HEIGHT 1080
#define NUM_PLANES 3

// near and far plane distances from RiftApp.h
#define NEAR_PLANE 0.01f
#define FAR_PLANE 1000.0f

struct ProjectionPlane {
	glm::vec3 pa, pb, pc, vr, vu, vn;
	glm::mat4 MT = glm::mat4(0.0f);
};

class CaveScene
{
	GLuint shaderId;
	GLuint basicShaderId;

	std::unique_ptr<Skybox> skybox;
	std::unique_ptr<Skybox> skyboxRight;

	// unscaled
	//std::array<glm::mat4, 3> instance_positions = {
	//	glm::translate(glm::vec3(0.0f, 0.0f, -0.5f)),
	//	glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::translate(glm::vec3(0.0f, 0.0f, -0.5f)),
	//	glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::translate(glm::vec3(0.0f, 0.0f, -0.5f))
	//};
	// scaled
	const float scale = 2.4f;
	std::array<glm::mat4, NUM_PLANES> instance_positions = {
		glm::rotate(glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::translate(glm::vec3(0.0f, 0.0f, -1.2f)) * glm::scale(glm::vec3(scale)),
		glm::rotate(glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::translate(glm::vec3(0.0f, 0.0f, -1.2f)) * glm::scale(glm::vec3(scale)),
		glm::rotate(glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::translate(glm::vec3(0.0f, 0.0f, -1.2f)) * glm::scale(glm::vec3(scale))
	};
	//std::unique_ptr<Plane> plane;
	std::unique_ptr<TexturedPlane> plane;
	std::array<ProjectionPlane, NUM_PLANES> projectionPlanes;

	//GLuint m_fbo;
	//GLuint m_texture;
	//GLuint m_rbo;

	GLuint m_fbo[NUM_PLANES];
	GLuint m_texture[NUM_PLANES];
	GLuint m_rbo[NUM_PLANES];

public:
	CaveScene() {
		// Shader Program 
		shaderId = LoadShaders("skybox.vert", "skybox.frag");
		basicShaderId = LoadShaders("basicColor.vert", "basicColor.frag");

		// 10m wide sky box: size doesn't matter though
		skybox = std::make_unique<Skybox>("skybox");
		skybox->toWorld = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));
		skyboxRight = std::make_unique<Skybox>("skybox_righteye");
		skyboxRight->toWorld = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));

		//plane = std::make_unique<Plane>();
		plane = std::make_unique<TexturedPlane>();

		// ----------------------------------------------------------------------------------------------
		// TODO: setup offscreen buffeers
		// generate framebuffer
		//glGenFramebuffers(1, &m_fbo);
		glGenFramebuffers(NUM_PLANES, m_fbo);
		glGenTextures(NUM_PLANES, m_texture);
		glGenRenderbuffers(NUM_PLANES, m_rbo);

		for (int i = 0; i < NUM_PLANES; i++) {
			glBindFramebuffer(GL_FRAMEBUFFER, m_fbo[i]);

			// generate texture to render to
			//glGenTextures(1, &m_texture);
			glBindTexture(GL_TEXTURE_2D, m_texture[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEX_WIDTH, TEX_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture[i], 0);
			glBindTexture(GL_TEXTURE_2D, 0);

			// depth buffer
			//glGenRenderbuffers(1, &m_rbo);
			glBindRenderbuffer(GL_RENDERBUFFER, m_rbo[i]);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, TEX_WIDTH, TEX_HEIGHT);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo[i]);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}


		// ----------------------------------------------------------------------------------------------
		// initialize projection plane information
		{
			using namespace PlaneData;
			for (int i = 0; i < NUM_PLANES; i++) {
				projectionPlanes[i].pa = instance_positions[i] * glm::vec4(bottomLeft, 1.0f);
				projectionPlanes[i].pb = instance_positions[i] * glm::vec4(bottomRight, 1.0f);
				projectionPlanes[i].pc = instance_positions[i] * glm::vec4(topLeft, 1.0f);
				projectionPlanes[i].vr = glm::normalize(projectionPlanes[i].pb - projectionPlanes[i].pa);
				projectionPlanes[i].vu = glm::normalize(projectionPlanes[i].pc - projectionPlanes[i].pa);
				projectionPlanes[i].vn = glm::normalize(glm::cross(projectionPlanes[i].vr, projectionPlanes[i].vu));
				projectionPlanes[i].MT = glm::transpose(glm::mat4(
					glm::vec4(projectionPlanes[i].vr, 0.0f),
					glm::vec4(projectionPlanes[i].vu, 0.0f),
					glm::vec4(projectionPlanes[i].vn, 0.0f),
					glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
				));
				//std::cerr << projectionPlanes[i].pa.x << " " << projectionPlanes[i].pa.y << " " << projectionPlanes[i].pa.z << " " << std::endl;
				//std::cerr << projectionPlanes[i].pb.x << " " << projectionPlanes[i].pb.y << " " << projectionPlanes[i].pb.z << " " << std::endl;
				//std::cerr << projectionPlanes[i].pc.x << " " << projectionPlanes[i].pc.y << " " << projectionPlanes[i].pc.z << " " << std::endl;
			}
		}
	}

	void render(
		const std::function<void(const glm::mat4&, const glm::mat4&, const ovrEyeType)> renderCave,
		const glm::mat4& projection,
		const glm::mat4& view,
		const ovrEyeType eye,
		const ovrPosef eyePose
	) {
		glm::vec3 pe = ovr::toGlm(eyePose.Position);
		glm::vec3 va, vb, vc;
		float d, l, r, b, t;
		glm::mat4 proj, projPrime, T;
		// calcuate correct projection

		for (int i = 0; i < NUM_PLANES; i++) {
			glBindFramebuffer(GL_FRAMEBUFFER, m_fbo[i]);
			glClearColor(0.1f, 0.1f, 1.0f, 1.0f);
			glClear(GL_DEPTH_BUFFER_BIT);

			// camera to plane vectors
			va = projectionPlanes[i].pa - pe;
			vb = projectionPlanes[i].pb - pe;
			vc = projectionPlanes[i].pc - pe;
			d = -glm::dot(projectionPlanes[i].vn, va); // distance

			// frustrum edges
			l = glm::dot(projectionPlanes[i].vr, va) * NEAR_PLANE / d;
			r = glm::dot(projectionPlanes[i].vr, vb) * NEAR_PLANE / d;
			b = glm::dot(projectionPlanes[i].vu, va) * NEAR_PLANE / d;
			t = glm::dot(projectionPlanes[i].vu, vc) * NEAR_PLANE / d;

			proj = glm::frustum(l, r, b, t, NEAR_PLANE, FAR_PLANE);
			T = glm::translate(-pe);
			projPrime = proj * projectionPlanes[i].MT * T;

			//renderCave(projPrime, view, eye);
			renderCave(projPrime, view, eye);
			// implicit call to glBindFramebuffer to eye bufferfs in passed in lambda function
		}

		renderCave(projection, view, eye);

		for (int i = 0; i < NUM_PLANES; i++) {
			plane->toWorld = instance_positions[i];
			plane->draw(projection, view, m_texture[i]);
		}

		if (eye == ovrEye_Left)
			skybox->draw(shaderId, projection, view);
		else
			skyboxRight->draw(shaderId, projection, view);
	}

	void render(
		const std::function<void(const glm::mat4&, const glm::mat4&, const ovrEyeType)> renderCave,
		const glm::mat4& projection,
		const glm::mat4& view,
		const ovrEyeType eye
	) {
		//glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		//glClearColor(0.1f, 0.1f, 1.0f, 1.0f);
		//glClear(GL_DEPTH_BUFFER_BIT);

		//renderCave(projection, view, eye);
		//// implicit call to glBindFramebuffer to eye bufferfs in passed in lambda function

		// //TODO: render the cave walls
		//for (auto & ip : instance_positions) {
		//	plane->toWorld = ip;
		//	//plane->draw(basicShaderId, projection, view);
		//	plane->draw(projection, view, m_texture);
		//}
		////plane->toWorld = instance_positions[0];
		////plane->draw(projection, view, m_texture);
		////renderCave(projection, view, eye);

		//if (eye == ovrEye_Left)
		//	skybox->draw(shaderId, projection, view);
		//else
		//	skyboxRight->draw(shaderId, projection, view);
	}
};