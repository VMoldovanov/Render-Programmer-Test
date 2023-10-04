#version 330 core
out vec4 FragColor;

in vec2 v_TCoord;

uniform sampler2D screenMap;
uniform sampler2D blurMap;
uniform sampler2D maskMap;

void main()
{
	float mask = texture(maskMap, v_TCoord).x;	
	vec3 blurTexel = texture(blurMap, v_TCoord).xyz;
	vec3 screenTexel = texture(screenMap, v_TCoord).xyz;
	
	FragColor = vec4(mix(screenTexel, blurTexel, mask), 1.0);
}