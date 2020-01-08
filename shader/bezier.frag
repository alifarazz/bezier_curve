#version 330 core

in vec2 FragUV;

out vec3 FragColor;

void main()
{
    if (FragUV.x * FragUV.x - FragUV.y > 0)
        FragColor = vec3(1, 0, 1);
    else
        FragColor = vec3(FragUV, 1);
}
