#version 430

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

out vec4 vpos;

void main() {
    vec4 pos = verts[gl_VertexID];
    vpos = view * pos;
    gl_Position = proj * vec4(vpos.xyz / (1 - vpos.w), 1);
    gl_PointSize = 5;
}
