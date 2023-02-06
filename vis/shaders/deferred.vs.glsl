#version 440 core

layout(std430, binding=1) buffer Positions {
    vec4 verts[];
};

layout(std140, binding=1) uniform Matrices {
    mat4 proj;
    mat4 view;
};

layout(std140, binding=2) uniform ModelMatrices {
    mat4 linear;
    vec4 translation;
};

layout(location=0) in ivec4 inds;
layout(location=1) in vec4 col;

out ivec4 vInds;
out vec4 vCol;

out gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
};

void main() {
    vInds = inds;
    vCol = col;

    vec4 pos = linear * verts[vInds.x] + translation;
    gl_Position = proj * vec4(pos.xyz, 1);
    gl_PointSize = 5;
}
