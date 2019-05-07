#pragma once

#include "Core.h"

class Plane
{
public:
	Plane();
	~Plane();

	glm::mat4 toWorld;

	void draw(GLuint shaderProgram, const glm::mat4& projection, const glm::mat4& view);

private:
	GLuint vertexBuffer, normalBuffer, VAO, ebo;
	GLuint uProjection, uModelview;
};

