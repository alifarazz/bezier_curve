#version 410 core

layout(vertices = 4) out;  // 4 points per patch
in vec2 vPos[];
out vec2 tcPos[];

void main() {
    tcPos[gl_InvocationID] = vPos[gl_InvocationID];
    if(gl_InvocationID == 0) { // levels only need to be set once per patch
        gl_TessLevelOuter[0] = 1; // we're only tessellating one line
        gl_TessLevelOuter[1] = 100; // tessellate the line into 100 segments
    }
}
