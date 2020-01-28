#version 440 core

layout(points) in;
layout(triangle_strip, max_vertices=4) out;

layout(std430, binding=1) buffer Positions {
    vec4 verts[];
};

layout(std140, binding=1) uniform Matrices {
    mat4 proj;
    mat4 view;
};

in ivec4 vInds[];

out vec4 pos;

out gl_PerVertex {
    vec4 gl_Position;
};

float unmix(float u, float v) {
    return (u) / (u - v);
}

void emit(vec4 v) {
    pos = v;
    gl_Position = proj * vec4(v.xyz, 1);
    EmitVertex();
}

void main() {
    vec4 pos4[4];
    for (int i = 0; i < 4; ++i) pos4[i] = view * verts[vInds[0][i]];

    int lo[4], L = 0;
    int hi[4], H = 0;

    float x = 0.7;

    for (int i = 0; i < 4; ++i) {
        if (pos4[i].w+x < 0) {
            lo[L++] = i;
        } else {
            hi[H++] = i;
        }
    }

    vec4 sect[4]; int S = 0;
    for (int l = 0; l < L; ++l)
    for (int h = H-1; h >=0; --h) {
        vec4 a = pos4[lo[l]];
        vec4 b = pos4[hi[h]];

        float t = unmix(a.w+x, b.w+x);
        sect[S++] = mix(a, b, t);
    }

    for (int s = 0; s < S; ++s) {
        emit(sect[s]);
    }

    EndPrimitive();
}
