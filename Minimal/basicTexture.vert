#version 330 core
uniform mat4 ProjectionMatrix = mat4(1);
uniform mat4 CameraMatrix = mat4(1);
uniform mat4 InstanceTransform = mat4(1);
uniform mat4 ModelMatrix = mat4(1);

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec3 texCoord;

//out vec3 ourColor;
out vec2 TexCoord;

void main()
{
   mat4 ViewXfm = CameraMatrix * InstanceTransform * ModelMatrix;
   gl_Position = ProjectionMatrix * ViewXfm * vec4(Position, 1.0);
//    vertNormal = Normal;
//	ourColor = aColor;
	TexCoord = vec2(texCoord.x, texCoord.y);
}