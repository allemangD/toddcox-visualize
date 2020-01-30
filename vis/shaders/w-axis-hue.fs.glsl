#version 430

in vec4 vpos;

out vec4 color;

void main() {
    float d = smoothstep(-2, 2, vpos.z);
    vec3 off = 1.04 * vec3(0, 2, 4) + 2 * vec3(vpos.w);
    color = vec4(d * cos(off), 1);
}
