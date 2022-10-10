#version 440

layout(location=0) uniform vec4 ucol;

layout(location=0) out vec4 col;

void main() {
    float d = 1.0 - gl_FragCoord.z;
    d = (d - 0.5) / 0.7 + 0.5;
    col = ucol;
    col.xyz *= d;
}
