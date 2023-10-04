#version 330 core
out vec2 v_TCoord;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTCoord;

void main()
{
	v_TCoord = aTCoord;
	gl_Position = vec4(aPos.x, aPos.y, 1.0, 1.0);
}