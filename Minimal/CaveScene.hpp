#pragma once

#include "Core.h"
#include "OvrHelper.h"

#include "BasicShader.h"
#include "Skybox.h"
#include "Plane.h"
#include "TexturedPlane.h"
#include "Shader.h"

#include <functional>

//#define TEX_WIDTH 1340
// these are determined by eyeSize gotten from ovr_GetFovTextureSize(...)
// alternatively, call glViewport again with our screen size, but don't want to deal with that right now.
#define TEX_WIDTH 1344
#define TEX_HEIGHT 1600
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
	//std::unique_ptr<Skybox> skyboxRight;

	// unscaled
	//std::array<glm::mat4, 3> instance_positions = {
	//	glm::translate(glm::vec3(0.0f, 0.0f, -0.5f)),
	//	glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::translate(glm::vec3(0.0f, 0.0f, -0.5f)),
	//	glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::translate(glm::vec3(0.0f, 0.0f, -0.5f))
	//};
	// scaled
	const float scale = 2.4f;
	std::array<glm::mat4, NUM_PLANES> instance_positions = {
		glm::rotate(glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::translate(glm::vec3(0.0f, 0.0f, -(scale/2))) * glm::scale(glm::vec3(scale)),
		glm::rotate(glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::translate(glm::vec3(0.0f, 0.0f, -(scale/2))) * glm::scale(glm::vec3(scale)),
		glm::rotate(glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::translate(glm::vec3(0.0f, 0.0f, -(scale/2))) * glm::scale(glm::vec3(scale))
	};
	//std::unique_ptr<Plane> plane;
	std::unique_ptr<TexturedPlane> plane;
	std::array<ProjectionPlane, NUM_PLANES> projectionPlanes;

	GLuint m_fbo[NUM_PLANES * 2];
	GLuint m_texture[NUM_PLANES * 2];
	GLuint m_rbo[NUM_PLANES * 2];

	// debug lines
	GLuint debug_vao[2], debug_vbo[2];
	//std::vector<glm::vec3> debug_vertices[2];
	glm::vec3 debug_vertices[2][12 * 3];

	Shader debug_shader = Shader("basicColor.vert", "basicColor.frag");

public:
	bool debugLines = false;

	CaveScene() {
		// Shader Program 
		shaderId = LoadShaders("skybox.vert", "skybox.frag");
		basicShaderId = LoadShaders("basicColor.vert", "basicColor.frag");

		// 10m wide sky box: size doesn't matter though
		//skybox = std::make_unique<Skybox>("skybox");
		skybox = std::make_unique<Skybox>("sb_frozendusk");
		skybox->toWorld = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));
		//skyboxRight = std::make_unique<Skybox>("skybox_righteye");
		//skyboxRight->toWorld = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));

		//plane = std::make_unique<Plane>();
		plane = std::make_unique<TexturedPlane>();

		// ----------------------------------------------------------------------------------------------
		// TODO: setup offscreen buffeers
		// generate framebuffer
		//glGenFramebuffers(1, &m_fbo);
		glGenFramebuffers(NUM_PLANES * 2, m_fbo);
		glGenTextures(NUM_PLANES * 2, m_texture);
		glGenRenderbuffers(NUM_PLANES * 2, m_rbo);
		//glGenFramebuffers(6, m_fbo);
		//glGenTextures(6, m_texture);
		//glGenRenderbuffers(6, m_rbo);

		for (int i = 0; i < NUM_PLANES * 2; i++) {
			std::cerr << i << std::endl;
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
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo[i]);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);

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

		// ----------------------------------------------------------------------------------------------
		// init debug object
		// create vertices
		{
			using namespace PlaneData;
			ovr::for_each_eye([&](ovrEyeType eye) {
				for (int i = 0; i < NUM_PLANES; i++) {
					glm::vec3 tr = instance_positions[i] * glm::vec4(topRight, 1.0f);
					glm::vec3 br = instance_positions[i] * glm::vec4(bottomRight, 1.0f);
					glm::vec3 bl = instance_positions[i] * glm::vec4(bottomLeft, 1.0f);
					glm::vec3 tl = instance_positions[i] * glm::vec4(topLeft, 1.0f);

					// top
					debug_vertices[eye][12 * i + 0] = glm::vec3(0.0f);
					debug_vertices[eye][12 * i + 1] = glm::vec3(tr);
					debug_vertices[eye][12 * i + 2] = glm::vec3(tl);

					// right
					debug_vertices[eye][12 * i + 3] = glm::vec3(0.0f);
					debug_vertices[eye][12 * i + 4] = glm::vec3(br);
					debug_vertices[eye][12 * i + 5] = glm::vec3(tr);

					// left
					debug_vertices[eye][12 * i + 6] = glm::vec3(0.0f);
					debug_vertices[eye][12 * i + 7] = glm::vec3(bl);
					debug_vertices[eye][12 * i + 8] = glm::vec3(br);

					// bottom
					debug_vertices[eye][12 * i + 9] = glm::vec3(0.0f);
					debug_vertices[eye][12 * i + 10] = glm::vec3(tl);
					debug_vertices[eye][12 * i + 11] = glm::vec3(bl);
				}
			});
		}

		glGenVertexArrays(2, debug_vao);
		glGenBuffers(2, debug_vbo);

		ovr::for_each_eye([&](ovrEyeType eye) {
			glBindVertexArray(debug_vao[eye]);
			glBindBuffer(GL_ARRAY_BUFFER, debug_vbo[eye]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(debug_vertices[eye]), debug_vertices[eye], GL_DYNAMIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		});
	}

	~CaveScene() {
		glDeleteVertexArrays(2, debug_vao);
		glDeleteBuffers(2, debug_vbo);
	}

	void render(
		const std::function<void(const glm::mat4&, const glm::mat4&, const ovrEyeType)> renderCave,
		const std::function<void()> returnFbo,
		const glm::mat4& projection,
		const glm::mat4& view,
		const ovrEyeType eye,
		const ovrPosef eyePose,
		bool updateScreen = true,
		bool controllerView = false
	) {
		glm::vec3 pe = ovr::toGlm(eyePose.Position);
		glm::vec3 va, vb, vc;
		float d, l, r, b, t;
		glm::mat4 proj, projPrime, T;
		// calcuate correct projection

		if (updateScreen) {
			for (int i = 0; i < NUM_PLANES; i++) {
				if (eye == ovrEye_Left)
					glBindFramebuffer(GL_FRAMEBUFFER, m_fbo[i]);
				else
					glBindFramebuffer(GL_FRAMEBUFFER, m_fbo[i + 3]);
				glClearColor(0.1f, 0.1f, 1.0f, 1.0f);
				glClear(GL_DEPTH_BUFFER_BIT);

				// camera to plane vectors
				va = projectionPlanes[i].pa - pe;
				vb = projectionPlanes[i].pb - pe;
				vc = projectionPlanes[i].pc - pe;
				d = -glm::dot(projectionPlanes[i].vn, va); // distance
				//d = -glm::dot(va, projectionPlanes[i].vn); // distance

				//std::cerr << (glm::dot(projectionPlanes[i].vr, va) + glm::dot(projectionPlanes[i].vr, vb)) << std::endl;
				//std::cerr << (glm::dot(projectionPlanes[i].vu, va) + glm::dot(projectionPlanes[i].vu, vc)) << std::endl;

				// frustrum edges
				l = glm::dot(projectionPlanes[i].vr, va) * NEAR_PLANE / d;
				r = glm::dot(projectionPlanes[i].vr, vb) * NEAR_PLANE / d;
				b = glm::dot(projectionPlanes[i].vu, va) * NEAR_PLANE / d;
				t = glm::dot(projectionPlanes[i].vu, vc) * NEAR_PLANE / d;

				proj = glm::frustum(l, r, b, t, NEAR_PLANE, FAR_PLANE);
				//T = glm::translate(-pe/2.4f); // idk why this is needed?
				T = glm::translate(-pe); // idk why this is needed?
				//projPrime = proj * projectionPlanes[i].MT * T;
				projPrime = proj * projectionPlanes[i].MT * T;
				//projPrime = (proj * projectionPlanes[i].MT) * T;

				glm::mat4 v = glm::inverse(ovr::toGlm(eyePose));
				//renderCave(projPrime, view, eye);
				//if (controllerView)
				//	//renderCave(projPrime, v, eye);
				//	renderCave(projPrime, glm::mat4(1.0f), eye);
				//	//renderCave(projPrime, view, eye);
				//else
				//	renderCave(projPrime, view, eye);
				renderCave(projPrime, glm::mat4(1.0f), eye);
				// implicit call to glBindFramebuffer to eye bufferfs in passed in lambda function
			}
		}

		returnFbo();

		std::cerr << eye << std::endl;
		for (int i = 0; i < NUM_PLANES; i++) {
			plane->toWorld = instance_positions[i];
			//plane->draw(projection, view, m_texture[i]);
			if (eye == ovrEye_Left) {
				plane->draw(projection, view, m_texture[i], ovr::toGlm(eyePose.Position));
				std::cerr << m_texture[i] << std::endl;
			}
			else {
				plane->draw(projection, view, m_texture[i + 3], ovr::toGlm(eyePose.Position));
				std::cerr << m_texture[i + 3] << std::endl;
			}
		}

		if (debugLines) {
			updateDebugVertices(eye, ovr::toGlm(eyePose.Position));
			updateDebugBuffer(eye);
			renderDebug(projection, view, eye);
		}

		skybox->draw(shaderId, projection, view);
		//if (eye == ovrEye_Left)
		//	skybox->draw(shaderId, projection, view);
		//else
		//	skyboxRight->draw(shaderId, projection, view);
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

	void updateDebugVertices(ovrEyeType eye, glm::vec3 eyePos = glm::vec3(0.0f)) {
		// create vertices
		{
			using namespace PlaneData;
			for (int i = 0; i < NUM_PLANES; i++) {
				// top
				debug_vertices[eye][12 * i + 0] = glm::vec3(eyePos);
				// right
				debug_vertices[eye][12 * i + 3] = glm::vec3(eyePos);
				// left
				debug_vertices[eye][12 * i + 6] = glm::vec3(eyePos);
				// bottom
				debug_vertices[eye][12 * i +  9] = glm::vec3(eyePos);
			}
		}
	}

	void updateDebugBuffer(ovrEyeType eye) {
			glBindVertexArray(debug_vao[eye]);
			glBindBuffer(GL_ARRAY_BUFFER, debug_vbo[eye]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(debug_vertices[eye]), debug_vertices[eye]);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
	}

	void renderDebug(const glm::mat4& projection, const glm::mat4& view, ovrEyeType eye) {
		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		ovr::for_each_eye([&](ovrEyeType eye) {
			debug_shader.use();
			// give info to shader
			// TODO: fix shader names
			debug_shader.setMat4("ProjectionMatrix", projection);
			debug_shader.setMat4("CameraMatrix", view);
			if (eye == ovrEye_Left)
				debug_shader.setVec4("Color", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
			else
				debug_shader.setVec4("Color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

			glBindVertexArray(debug_vao[eye]);
			glDrawArrays(GL_TRIANGLES, 0, 12 * 3);
			glBindVertexArray(0);
		});
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_CULL_FACE);
	}

	void toggleLightingMode() {
		plane->lightingMode = plane->lightingModeMap.at(plane->lightingMode);
	}
};