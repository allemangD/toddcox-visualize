#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <tc/groups.hpp>

#include "util.hpp"
#include "mirror.hpp"
#include "geometry.hpp"

#ifdef _WIN32
extern "C" {
__attribute__((unused)) __declspec(dllexport) int NvOptimusEnablement = 0x00000001;
}
#endif

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
    glfwSwapInterval(0);

    //endregion

    std::cout << utilInfo();

    GLuint pgm;

    try {
        GLuint vs = utilCompileFiles(GL_VERTEX_SHADER, {"shaders/ortho.vs.glsl"});
        GLuint fs = utilCompileFiles(GL_FRAGMENT_SHADER, {"shaders/w-axis-hue.fs.glsl"});

        pgm = utilLinkProgram({vs, fs});
    } catch (const gl_error &e) {
        std::cerr << e.what() << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    auto group = tc::group::H(4);
    auto res = group.solve();
    auto mirrors = mirror(group);

    auto corners = plane_intersections(mirrors);
    auto start = barycentric(corners, {1.00, 1.00, 1.00, 1.00});
    auto points = res.path.walk<glm::vec4, glm::vec4>(start, mirrors, reflect);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * points.size(), &points[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, false, 0, nullptr);

    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(pgm);
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_POINT_SMOOTH);
        glEnable(GL_DEPTH_TEST);

        //region uniforms
        auto aspect = (float) width / (float) height;
        auto pheight = 1.4f;
        auto pwidth = aspect * pheight;
        glm::mat4 proj = glm::ortho(-pwidth, pwidth, -pheight, pheight, -100.0f, 100.0f);
        glUniformMatrix4fv(0, 1, false, glm::value_ptr(proj));

        auto t = (float) glfwGetTime() / 5;
        auto view = glm::identity<glm::mat4>();
        view *= utilRotate(0, 1, t * 0.7f);
        view *= utilRotate(0, 2, t * 0.8f);
        view *= utilRotate(0, 3, t * 1.0f);
        view *= utilRotate(1, 2, -t * 1.1f);
        view *= utilRotate(1, 3, -t * 0.3f);
        view *= utilRotate(2, 3, -t * 1.2f);
        glUniformMatrix4fv(1, 1, false, glm::value_ptr(view));
        //endregion

        glUniform3f(2, 1.0f, 1.0f, 1.0f);
        glDrawArrays(GL_POINTS, 0, points.size());

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}
