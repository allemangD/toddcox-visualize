#version 440

layout(location=1) uniform float time;
layout(location=2) uniform mat4 proj;

layout(location=0) in vec4 pos;

void main() {
    float c2 = cos(time * 0.2);
    float s2 = sin(time * 0.2);
    float c3 = cos(time * 0.3);
    float s3 = sin(time * 0.3);
    float c4 = cos(time * 0.25);
    float s4 = sin(time * 0.25);

    mat4 r1 = mat4(
    c2, -s2, 0.0, 0.0,
    s2, c2, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
    );

    mat4 r2 = mat4(
    c3, 0.0, -s3, 0.0,
    0.0, 1.0, 0.0, 0.0,
    s3, 0.0, c3, 0.0,
    0.0, 0.0, 0.0, 1.0
    );

    mat4 r3 = mat4(
    1.0, 0.0, 0.0, 0.0,
    0.0, c4, 0.0, -s4,
    0.0, 0.0, 1.0, 0.0,
    0.0, s4, 0.0, c4
    );

    vec4 pos = r2 * r1 * r3 * pos;

    gl_Position = proj * vec4(pos.xyz / (1 - pos.w), 1.0);
}
