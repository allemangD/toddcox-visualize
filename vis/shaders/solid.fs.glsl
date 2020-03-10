#version 430

//layout(location=2) uniform vec3 c;

in vec4 pos;
in vec4 col;

out vec4 color;

void main() {
    color = col;
//    float d = smoothstep(-2, 2, pos.z);
//    color = vec4(c * d, 1);
}
