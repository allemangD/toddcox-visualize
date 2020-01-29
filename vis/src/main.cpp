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
//    GeomGen<4> gg4(group);
//    GeomGen<3> gg3(group);
//    auto res = gg4.solve();
    auto res = group.solve();
    auto mirrors = mirror(group);

    auto corners = plane_intersections(mirrors);
//    auto start = barycentric(corners, {1.0f, 1.0f, 1.0f, 1.0f});
    auto start = barycentric(corners, {1.00f, 0.2f, 0.1f, 0.05f});
//    auto start = barycentric(corners, {0.05f, 0.1f, 0.2f, 1.00f});
    auto points = res.path.walk<glm::vec4, glm::vec4>(start, mirrors, reflect);

    auto g_gens = gens(group);

    std::vector<GLuint> vaos;
    std::vector<GLuint> ibos;
    std::vector<unsigned> counts;

    auto combos = Combos(g_gens, 3);
//    std::vector<std::vector<int>> chosen = {
//        {1, 2, 3},
//        {0, 2, 3},
//    };
    auto chosen = combos;

    for (const auto& sg_gens : chosen) {
        const Mesh<4> &base = triangulate<4>(group, sg_gens);
        const auto &s = base;
//         s = tile(context, g_gens, sg_gens, base);

        GLuint vao = utilCreateVertexArray();
        GLuint ibo = utilCreateBuffer();
        unsigned count = s.size();

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, ibo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Primitive<4>) * count, &s.prims[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribIPointer(0, 4, GL_INT, 0, nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        vaos.push_back(vao);
        ibos.push_back(ibo);
        counts.push_back(count);
    }
    //endregion

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * points.size(), &points[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint ubo;
    glGenBuffers(1, &ubo);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vbo);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo);

    glBindVertexArray(0);

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

        for (int i = 0; i < vaos.size(); ++i) {
            auto c = glm::mix(
                glm::vec3(.3, .2, .5),
                glm::vec3(.9, .9, .95),
                (float) (i) / (vaos.size() - 1.f)
            );

            glBindProgramPipeline(pipe);
            glBindVertexArray(vaos[i]);
            glProgramUniform3f(fs, 2, c.r, c.g, c.b);
            glDrawArrays(GL_POINTS, 0, counts[i] / 4);
        }

        glBindProgramPipeline(0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}
