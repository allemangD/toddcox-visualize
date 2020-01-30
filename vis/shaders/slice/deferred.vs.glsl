#version 440 core

layout(std430, binding=1) buffer Positions {
    vec4 verts[];
};

layout(std140, binding=1) uniform Matrices {
    mat4 proj;
    mat4 view;
};

layout(location=0) in ivec4 inds;

out ivec4 vInds;

out gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
};

void main() {
    vInds = inds;

    vec4 pos = view * verts[vInds.x];
    gl_Position = proj * vec4(pos.xyz, 1);
    gl_PointSize = 5;
}
