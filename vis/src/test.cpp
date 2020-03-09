#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>

#include <tc/groups.hpp>

#include "util.hpp"
#include "mirror.hpp"
#include "geometry.hpp"

#include <cgl/render.hpp>

#ifdef _WIN32
extern "C" {
__attribute__((unused)) __declspec(dllexport) int NvOptimusEnablement = 0x00000001;
}
#endif

struct globals {
    glm::mat4 proj = glm::identity<glm::mat4>();
    float time = 0;
};

void run(GLFWwindow *window) {
    auto direct = cgl::pgm::vert::file("shaders/test/direct.glsl");
    auto white = cgl::pgm::frag::file("shaders/test/white.glsl");

    auto pipe = cgl::pipeline();
    pipe.stage(direct).stage(white);

    auto vbo = cgl::buffer<float>({
        -0.5, +0.866, 0, 1,
        -0.5, -0.866, 0, 1,
        +1.0, +0.000, 0, 1,
    });

    auto global_ubo = cgl::buffer<globals>();
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, global_ubo);

    auto vao = cgl::vertexarray();
    vao.bound([&]() {
        vbo.bound(GL_ARRAY_BUFFER, [&]() {
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
        });
    });

    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto aspect = (float) width / (float) height;
        globals data;
        data.time = (float) glfwGetTime();
        data.proj = glm::ortho(-aspect, aspect, -1.0f, 1.0f);
        global_ubo.put(data);

        pipe.bound([&]() {
            vao.bound([&]() {
                glDrawArrays(GL_TRIANGLES, 0, 3);
            });
        });

        glfwSwapBuffers(window);

        glfwPollEvents();
    }
}

int main(int argc, char *argv[]) {
    //region init window
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
    glfwSwapInterval(1);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);
    //endregion

    std::cout << utilInfo();

    run(window);

    glfwTerminate();
    return EXIT_SUCCESS;
}
