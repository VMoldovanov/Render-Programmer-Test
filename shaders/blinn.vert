#version 330 core
out vec3 v_Pos;
out vec2 v_TCoord;
out vec3 v_Normal;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

uniform mat4 u_matProjection;
uniform mat4 u_matView;
uniform mat4 u_matModel;
uniform mat3 u_matNormal;

void main()
{	
	v_Pos = vec3(u_matModel * vec4(aPos, 1.0));
	v_TCoord = aTCoord;
	v_Normal = u_matNormal * aNormal;
	gl_Position = u_matProjection * u_matView * vec4(v_Pos, 1.0);
}