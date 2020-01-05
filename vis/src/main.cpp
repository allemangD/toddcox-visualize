#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <tc/groups.h>
#include <tc/solver.h>

#include "geom.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#ifdef _WIN32
extern "C" {
__attribute__((unused)) __declspec(dllexport) int NvOptimusEnablement = 0x00000001;
}
#endif

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

int main(int argc, char *argv[]) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return EXIT_FAILURE;
    }

    auto window = glfwCreateWindow(
        1920, 1080,
        "Coset Visualization",
        nullptr, nullptr);

    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(0);

    std::cout
        << "Graphics Information:" << std::endl
        << "    Vendor:          " << glGetString(GL_VENDOR) << std::endl
        << "    Renderer:        " << glGetString(GL_RENDERER) << std::endl
        << "    OpenGL version:  " << glGetString(GL_VERSION) << std::endl
        << "    Shading version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    auto group = tc::group::B(4);
    auto res = tc::solve(group);
    std::cout
        << "Coset Solution Test:" << std::endl
        << "    Group: " << group.name << std::endl
        << "    Order: " << res.len << std::endl;

    auto mirrors = mirror(group);
    std::cout << "Mirrors:" << std::endl;
    for (const auto &mirror : mirrors) {
        std::cout << "    " << glm::to_string(mirror) << std::endl;
    }

    auto corners = plane_intersections(mirrors);

    auto points = std::vector<glm::vec4>(res.size());
    points[0] = barycentric(corners, {1, 0, 0, 0});

    for (int i = 1; i < res.size(); i++) {
        auto action = res.path[i];
        points[i] = reflect(points[action.coset], mirrors[action.gen]);
    }

    std::cout << "Points:" << std::endl;
    for (const auto &point : points) {
        std::cout << "    " << to_string(point) << std::endl;
    }

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    utilShaderSource(vs, {
        "#version 430\n",

        "layout(location=0) uniform mat4 proj;"
        ""
        "void main() {"
        "   int i = gl_VertexID;"
        "   gl_Position = proj * vec4(i % 2, i / 2, 0, 1);"
        "}"
    });
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    utilShaderSource(fs, {
        "#version 430\n",

        "out vec4 color;"
        ""
        "void main() {"
        "   color = vec4(1);"
        "}"
    });

    GLuint pgm = glCreateProgram();
    glAttachShader(pgm, vs);
    glAttachShader(pgm, fs);
    glLinkProgram(pgm);

    GLint status;

    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
    if (!status) {
        std::cerr << utilShaderInfoLog(vs) << "\n=========\n" << std::endl;
    }

    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
    if (!status) {
        std::cerr << utilShaderInfoLog(fs) << "\n=========\n" << std::endl;
    }

    glGetProgramiv(pgm, GL_LINK_STATUS, &status);
    if (!status) {
        std::cerr << utilProgramInfoLog(pgm) << "\n=========\n" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    auto aspect = (float) width / (float) height;

    glm::mat4 proj = glm::ortho(-aspect, aspect, -1.f, 1.f);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(pgm);
        glUniformMatrix4fv(0, 1, false, glm::value_ptr(proj));
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}
