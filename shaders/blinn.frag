#version 330 core
out vec4 FragColor;

in vec3 v_Pos;
in vec2 v_TCoord;
in vec3 v_Normal;

uniform sampler2D u_diffuseMap;
uniform vec3 u_lightPos;
uniform vec3 u_viewPos; 

vec3 ShadingBlinn(vec3 texel)
{
    // компонент рассеянного света
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * texel;
  	
    // компонент направленного света от источника
    vec3 normal = normalize(v_Normal);
    vec3 lightDir = normalize(u_lightPos - v_Pos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * texel;
    
    // компонент бликовой составляющей от направленного источника
    float specularStrength = 0.5;
    vec3 viewDir = normalize(u_viewPos - v_Pos);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
	vec3 specular = vec3(specularStrength * spec);
		
    return (ambient + diffuse + specular);
}

void main()
{
   vec3 texel = texture(u_diffuseMap, v_TCoord).xyz;
   FragColor = vec4(ShadingBlinn(texel), 1.0);
}