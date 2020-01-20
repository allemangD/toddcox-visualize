#version 430

layout(location=1) uniform mat4 view;

layout(location=0) in vec4 pos;

out vec4 gpos;

void main() {
    gpos = view * pos;
    gl_PointSize = 5;
}
