#version 430

#define SUBS 20

layout(lines) in;
layout(line_strip, max_vertices = SUBS) out;

layout(location=0) uniform mat4 proj;

in vec4 gpos[];
out vec4 vpos;

vec4 stereo(vec4 v) {
    return vec4(v.xyz / (1 - v.w), 1);
}

void main() {
    for (int i = 0; i < SUBS; i++) {
        vpos = mix(gpos[0], gpos[1], i * 1.0f / (SUBS - 1));
        vpos = normalize(vpos);
        gl_Position = proj * stereo(vpos);
        EmitVertex();
    }
    EndPrimitive();
}
