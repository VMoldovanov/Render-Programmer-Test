#version 330 core
out vec4 FragColor;

in vec2 v_TCoord;

uniform sampler2D screenMap;
uniform sampler2D blurMap;

uniform vec4 u_rectScreen;

void main()
{
	vec3 blurTexel = texture(blurMap, v_TCoord).xyz;
	vec3 screenTexel = texture(screenMap, v_TCoord).xyz;
	
	if (gl_FragCoord.x >= u_rectScreen.x && gl_FragCoord.x <= (u_rectScreen.x + u_rectScreen.z) &&
		gl_FragCoord.y >= u_rectScreen.y && gl_FragCoord.y <= (u_rectScreen.y + u_rectScreen.w))
		FragColor = vec4(blurTexel, 1.0);
	else
		FragColor = vec4(screenTexel, 1.0);
}