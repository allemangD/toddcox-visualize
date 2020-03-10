#pragma once

#include <cerrno>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <glad/glad.h>

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

std::string utilReadFile(const std::string &filename) {
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (in) {
        std::ostringstream contents;
        contents << in.rdbuf();
        in.close();
        return (contents.str());
    }
    throw std::system_error(errno, std::generic_category());
}
