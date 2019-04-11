#version 410 core

uniform vec4 Color = vec4(1);

in vec3 vertNormal;
out vec4 fragColor;

void main(void) {
    fragColor = Color;
}
