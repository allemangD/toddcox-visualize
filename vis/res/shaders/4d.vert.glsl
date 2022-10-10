#version 440

layout(location=1) uniform float time;
layout(location=2) uniform mat4 proj;
layout(location=3) uniform mat4 rot;

layout(location=0) in vec4 pos;

void main() {
    vec4 pos = rot * pos;
    gl_Position = proj * vec4(pos.xyz / (1 - pos.w), 1.0);
}
