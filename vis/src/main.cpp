#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <tc/groups.h>
#include <tc/solver.h>

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

    auto group = tc::group::A(5);
    auto res = tc::solve(group);
    std::cout
        << "Coset Solution Test:" << std::endl
        << "    Group: " << group.name << std::endl
        << "    Order: " << res.len << std::endl;

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}