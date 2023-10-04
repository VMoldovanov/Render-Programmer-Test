#version 330 core
out vec4 FragColor;

in vec2 v_TCoord;
in vec3 v_Shading;

uniform sampler2D u_diffuseMap;

void main()
{
   vec3 texel = texture(u_diffuseMap, v_TCoord).xyz;
   FragColor = vec4(texel * v_Shading, 1.0);
}