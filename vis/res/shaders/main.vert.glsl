#version 440

layout(location=1) uniform float time;
layout(location=2) uniform mat4 proj;
layout(location=3) uniform mat4 view;

layout(location=0) in vec4 pos;

void main() {
    vec3 pos3 = (view * pos).xyz;
    
    gl_Position = proj * vec4(pos3, 1.0);
}
