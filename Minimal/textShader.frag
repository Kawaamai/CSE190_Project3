#version 410 core

uniform sampler2D text;
uniform vec3 textColor;

in vec2 textCoords;
out vec4 color;

void main() {
	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, textCoords).r);
	color = vec4(textColor, 1.0) * sampled;
}
