#version 410 core
out vec4 FragColor;

in vec2 TexCoord;

uniform vec3 eyePos = vec3(0.0f);
uniform vec3 fragNormal = vec3(0.0f);
uniform sampler2D tex;

void main()
{
	
	FragColor = texture(tex, TexCoord);
}
