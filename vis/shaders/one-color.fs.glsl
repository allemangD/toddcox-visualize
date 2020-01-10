#version 430

layout(location=2) uniform vec3 c;

in vec4 vpos;

out vec4 color;

void main() {
    float d = smoothstep(-2, 2, vpos.z);
    color = vec4(c * d, 1);
}
