#version 430

layout(std140, binding=0) uniform globals {
    mat4 proj;
    float time;
};

out vec4 diffuse;

void main() {
    vec3 hue = vec3(time) + vec3(0, 2, 4) * 3.1415 / 6.0;
    vec3 rgb = cos(hue) * 0.3 + vec3(0.7);

    diffuse = vec4(rgb, 1);
}
