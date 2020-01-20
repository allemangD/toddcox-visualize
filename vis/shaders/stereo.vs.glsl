#version 430

layout(location=0) uniform mat4 proj;
layout(location=1) uniform mat4 view;

layout(location=0) in vec4 pos;

out vec4 vpos;

void main() {
    int i = gl_VertexID;
    vpos = view * pos;
     gl_Position = proj * vec4(vpos.xyz / (1 - vpos.w), 1);
    gl_PointSize = 5 * smoothstep(-2, 2, gl_Position.z);
}
