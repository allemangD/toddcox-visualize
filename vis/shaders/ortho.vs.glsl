#version 430

out gl_PerVertex {
    vec4 gl_Position;
};

layout(location=0) uniform mat4 proj;
layout(location=1) uniform mat4 view;

layout(location=0) in vec4 pos;

out vec4 vpos;

void main() {
    vpos = view * pos;
    gl_Position = proj * vec4(vpos.xyz / (1), 1);
}
