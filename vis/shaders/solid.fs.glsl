#version 430

layout(location=2) uniform vec3 c;

in vec4 pos;

out vec4 color;

void main() {
    if (pos.w > 0.8) discard;

    float d = smoothstep(-2, 2, pos.z);
    color = vec4(c * d, 1);
}
