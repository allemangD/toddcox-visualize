#version 440

layout(location=1) uniform float time;
layout(location=2) uniform mat4 proj;
layout(location=3) uniform mat4 rot;
layout(location=0) in vec3 pos;

void main() {
    mat3 rot3 = mat3(rot);
    vec3 pos3 = rot3 * pos;

    gl_Position = proj * vec4(pos3, 1.0);
}
