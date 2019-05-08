#pragma once

#include "Core.h"
#include "Shader.h"

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

