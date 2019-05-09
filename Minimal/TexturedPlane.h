#pragma once

#include "Core.h"
#include "Shader.h"

namespace PlaneData {
	const float vertices[] = {
		 0.5f,  0.5f, 0.0f, // top right
		 0.5f, -0.5f, 0.0f, // bottom right
		-0.5f, -0.5f, 0.0f, // bottom left
		-0.5f,  0.5f, 0.0f  // top left
	};

	const float texCoords[] = {
		1.0f, 1.0f, // top right
		1.0f, 0.0f, // bottom right
		0.0f, 0.0f, // bottom left
		0.0f, 1.0f  // top left
	};

	const GLuint indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	const glm::vec3 topRight    = glm::vec3( 0.5f,  0.5f, 0.0f);
	const glm::vec3 bottomRight = glm::vec3( 0.5f, -0.5f, 0.0f);
	const glm::vec3 bottomLeft  = glm::vec3(-0.5f, -0.5f, 0.0f);
	const glm::vec3 topLeft     = glm::vec3(-0.5f,  0.5f, 0.0f);
}

class TexturedPlane
{
public:
	TexturedPlane();
	~TexturedPlane();

	glm::mat4 toWorld;
	GLuint textureId;
	Shader shader = Shader("basicTexture.vert", "basicTexture.frag");

	void draw(const glm::mat4& projection, const glm::mat4& view, GLuint textureId);
	void draw(GLuint shaderProgram, const glm::mat4& projection, const glm::mat4& view);

private:
	GLuint vbo, vao, ebo, tbo;
};

