#version 430

layout(std140, binding=0) uniform globals {
    mat4 proj;
    float time;
};

layout(location=0) in vec4 pos;

out gl_PerVertex{
    vec4 gl_Position;
};

void main() {
    gl_Position = proj * pos;
}
