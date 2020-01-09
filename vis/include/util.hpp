#pragma once

#include <string>
#include <vector>
#include <glad/glad.h>

#include <sstream>

void utilShaderSource(GLuint shader, const std::vector<std::string> &sources) {
    char const *ptrs[sources.size()];
    for (size_t i = 0; i < sources.size(); ++i) {
        ptrs[i] = sources[i].c_str();
    }
    glShaderSource(shader, sources.size(), ptrs, nullptr);
}

std::string utilShaderInfoLog(GLuint shader) {
    int len;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    char buffer[len];
    glGetShaderInfoLog(shader, len, nullptr, buffer);
    return std::string(buffer);
}

std::string utilProgramInfoLog(GLuint program) {
    int len;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
    char buffer[len];
    glGetProgramInfoLog(program, len, nullptr, buffer);
    return std::string(buffer);
}

std::string utilInfo() {
    std::stringstream ss;
    ss
        << "Graphics Information:" << std::endl
        << "    Vendor:          " << glGetString(GL_VENDOR) << std::endl
        << "    Renderer:        " << glGetString(GL_RENDERER) << std::endl
        << "    OpenGL version:  " << glGetString(GL_VERSION) << std::endl
        << "    Shading version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    return ss.str();
}