#version 410 core

layout(isolines) in;
in vec2 tcPos[];

vec2 bezier2(vec2 a, vec2 b, float t) {
    return mix(a, b, t);
}
vec2 bezier3(vec2 a, vec2 b, vec2 c, float t) {
    return mix(bezier2(a, b, t), bezier2(b, c, t), t);
}

vec2 bezier4(vec2 a, vec2 b, vec2 c, vec2 d, float t) {
    return mix(bezier3(a, b, c, t), bezier3(b, c, d, t), t);
}

void main() {
    float t = gl_TessCoord.x;
    vec2 ePos = bezier4(tcPos[0], tcPos[1], tcPos[2], tcPos[3], t);
    gl_Position = vec4(ePos, 0, 1);
}
