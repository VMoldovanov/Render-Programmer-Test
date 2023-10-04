#version 330 core
out vec2 v_TCoord;
out vec3 v_Shading;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

uniform mat4 u_matProjection;
uniform mat4 u_matView;
uniform mat4 u_matModel;
uniform mat3 u_matNormal;

uniform vec3 u_lightPos;
uniform vec3 u_viewPos; 

vec3 ShadingGouraud(vec3 vertexPos, vec3 vertexNormal)
{
    // компонент рассеянного света
    float ambientStrength = 0.2;
    vec3 ambient = vec3(ambientStrength);
  	
    // компонент направленного света от источника
    vec3 normal = normalize(vertexNormal);
    vec3 lightDir = normalize(u_lightPos - vertexPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = vec3(diff);
    
    // компонент бликовой составляющей от направленного источника
    float specularStrength = 0.5;
    vec3 viewDir = normalize(u_viewPos - vertexPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
	vec3 specular = vec3(specularStrength * spec);
		
    return (ambient + diffuse + specular);	
}

void main()
{	
	vec3 vertexPos = vec3(u_matModel * vec4(aPos, 1.0));
	vec3 vertexNormal = u_matNormal * aNormal;
	gl_Position = u_matProjection * u_matView * vec4(vertexPos, 1.0);
	
	// вычисляем затенение по Гуро
	v_Shading = ShadingGouraud(vertexPos, vertexNormal);
	v_TCoord = aTCoord;
}