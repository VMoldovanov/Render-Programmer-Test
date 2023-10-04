#version 330 core
out vec4 FragColor;

in vec2 v_TCoord;

uniform sampler2D screenMap;

void main()
{
	FragColor = vec4(texture(screenMap, v_TCoord).xyz, 1.0);
}