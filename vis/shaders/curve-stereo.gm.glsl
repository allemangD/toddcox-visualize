#version 430

#define SUBS 20

layout(lines) in;
layout(line_strip, max_vertices = SUBS) out;

layout(std430, binding=1) buffer Positions {
    vec4 verts[];
};

layout(std140, binding=1) uniform Matrices {
    mat4 proj;
    mat4 view;
};

out gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
};

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
