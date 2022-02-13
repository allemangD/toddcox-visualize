#version 440

layout(location=1) uniform float time;
layout(location=2) uniform mat4 proj;
layout(location=0) in vec3 pos;

void main() {
    float c2 = cos(time * 0.2);
    float s2 = sin(time * 0.2);
    float c3 = cos(time * 0.3);
    float s3 = sin(time * 0.3);

    mat4 r1 = mat4(c2, -s2, 0.0, 0.0, s2, c2, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);

    mat4 r2 = mat4(c3, 0.0, -s3, 0.0, 0.0, 1.0, 0.0, 0.0, s3, 0.0, c3, 0.0, 0.0, 0.0, 0.0, 1.0);

    gl_Position = proj * r2 * r1 * vec4(pos, 1.0);
}
