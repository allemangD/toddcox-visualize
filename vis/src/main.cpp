#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <tc/groups.h>
#include <tc/solver.h>

#include "geom.h"

#include <glm/gtx/string_cast.hpp>

#ifdef _WIN32
extern "C" {
__attribute__((unused)) __declspec(dllexport) int NvOptimusEnablement = 0x00000001;
}
#endif

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

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}
