#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
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

struct Matrices {
    glm::mat4 proj;
    glm::mat4 view;
};

Matrices build(GLFWwindow *window, float st) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    auto aspect = (float) width / (float) height;
    auto pheight = 1.4f;
    auto pwidth = aspect * pheight;
    glm::mat4 proj = glm::ortho(-pwidth, pwidth, -pheight, pheight, -10.0f, 10.0f);

    auto view = glm::identity<glm::mat4>();
    view *= utilRotate(0, 1, st * .40f);
    view *= utilRotate(0, 2, st * .20f);
    view *= utilRotate(0, 3, st * 1.30f);
    view *= utilRotate(1, 2, st * .50f);
    view *= utilRotate(1, 3, st * .25f);
    view *= utilRotate(2, 3, st * 1.42f);

    return {proj, view};
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
    glfwSwapInterval(0);
    //endregion

    std::cout << utilInfo();

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    //region shaders
    GLuint pipe;
    glCreateProgramPipelines(1, &pipe);

    GLuint vs, gm, fs;

    try {
        vs = utilCreateShaderProgramFile(GL_VERTEX_SHADER, {"shaders/4d/4d.vs.glsl"});
        gm = utilCreateShaderProgramFile(GL_GEOMETRY_SHADER, {"shaders/4d/4d.gm.glsl"});
        fs = utilCreateShaderProgramFile(GL_FRAGMENT_SHADER, {"shaders/one-color.fs.glsl"});

        glUseProgramStages(pipe, GL_VERTEX_SHADER_BIT, vs);
        glUseProgramStages(pipe, GL_GEOMETRY_SHADER_BIT, gm);
        glUseProgramStages(pipe, GL_FRAGMENT_SHADER_BIT, fs);
    } catch (const gl_error &e) {
        std::cerr << e.what() << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    //endregion

    //region points
    auto group = tc::group::H(4);
    GeomGen gg(group);
    auto res = gg.solve();
    auto mirrors = mirror(group);

    auto corners = plane_intersections(mirrors);
    auto start = barycentric(corners, {1.00f, 0.1f, 0.01f, 0.05f});
    auto points = res.path.walk<glm::vec4, glm::vec4>(start, mirrors, reflect);

    auto g_gens = gg.group_gens();
    std::vector<int> sg_gens = {0, 1, 2};
    const std::vector<int> simps = gg.tile(g_gens, sg_gens, gg.triangulate(sg_gens)).vals;
    //endregion

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * points.size(), &points[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint ubo;
    glGenBuffers(1, &ubo);

    GLuint ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ARRAY_BUFFER, ibo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(int) * simps.size(), &simps[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vbo);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, ibo);
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 4, GL_INT, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    std::cout << points.size() << " points" << std::endl;
    std::cout << simps.size() << " simplexes" << std::endl;

    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto st = (float) glfwGetTime() / 8;
        Matrices mats = build(window, st);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(mats), &mats, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindVertexArray(vao);
        glBindProgramPipeline(pipe);

        glProgramUniform3f(fs, 2, 1.0f, 1.0f, 1.0f);
        glDrawArrays(GL_POINTS, 0, simps.size() / 4);

        glBindProgramPipeline(0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}
