#version 410 core

uniform mat4 ProjectionMatrix = mat4(1);
uniform mat4 CameraMatrix = mat4(1);
uniform mat4 InstanceTransform = mat4(1);
uniform mat4 ModelMatrix = mat4(1);

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;

out vec3 fragNormal;
out vec3 fragPos;

void main(void) {
   mat4 ViewXfm = CameraMatrix * InstanceTransform * ModelMatrix;
   fragNormal = mat3(transpose(inverse(InstanceTransform * ModelMatrix))) * Normal;
   fragPos = vec3(InstanceTransform * ModelMatrix * vec4(Position, 1.0));

   gl_Position = ProjectionMatrix * ViewXfm * vec4(Position, 1.0);
}
