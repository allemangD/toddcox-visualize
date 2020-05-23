#version 430

layout(location=2) uniform vec3 col;

layout(location=0) in vec4 pos;
layout(location=2) in vec3 normal;

out vec4 color;

void main() {
    float depth = .1 + .9 * smoothstep(-2, 2, pos.z);
    float bright = abs(dot(normal, normalize(vec3(-0.6, 1, 2))));
    bright = .6 + .3 * bright * depth;

    color = vec4(col, 1);
    color.xyz *= bright;
}
