#version 410 core

in vec2 vPos;
out vec3 color;

void main() {
    if (distance(gl_PointCoord, vPos) < .2)
       color = vec3(1);
}
