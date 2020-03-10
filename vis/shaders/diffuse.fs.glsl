#version 430

layout(location=1) in vec4 col;
layout(location=2) in vec3 normal;

out vec4 color;

void main() {
    float bright = dot(normal, normalize(vec3(-0.6, 1, 2)));
    bright = .6 + .3 * bright;

    color = col;
    color.xyz *= bright;
}
