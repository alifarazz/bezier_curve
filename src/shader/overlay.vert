#version 410 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;
out vec2 vPos;
out vec3 vColor;

void main () {
    gl_Position = vec4(aPos, 0, 1);
    vPos = aPos;
    vColor = aColor;
}
