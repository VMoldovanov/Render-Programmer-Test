#version 330 core
out vec4 FragColor;

in vec2 v_TCoord;

uniform sampler2D screenMap;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
uniform bool u_horizontal;

vec3 BlurGauss()
{             
	// получаем размер текселя
    vec2 offset = 1.0 / textureSize(screenMap, 0);
	// вклад текущего фрагмента
    vec3 result = texture(screenMap, v_TCoord).xyz * weight[0];
	// учитываем направление размытия
    if(u_horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(screenMap, v_TCoord + vec2(offset.x * i, 0.0)).xyz * weight[i];
            result += texture(screenMap, v_TCoord - vec2(offset.x * i, 0.0)).xyz * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(screenMap, v_TCoord + vec2(0.0, offset.y * i)).xyz * weight[i];
            result += texture(screenMap, v_TCoord - vec2(0.0, offset.y * i)).xyz * weight[i];
        }
    }
    return result;
}

void main()
{
	FragColor = vec4(BlurGauss(), 1.0);
}