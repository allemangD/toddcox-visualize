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

float floatmod(float x, float m) {
    if (x < 0) {
        x += (float)((int)((-x)/m) + 1)*m;
    }
    return x - ((float)((int)(x / m)))*m;
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

    GLuint pgm;

    try {
//        GLuint vs = utilCompileFiles(GL_VERTEX_SHADER, {"shaders/ortho.vs.glsl"});
//        GLuint vs = utilCompileFiles(GL_VERTEX_SHADER, {"shaders/stereo.vs.glsl"});
//        GLuint fs = utilCompileFiles(GL_FRAGMENT_SHADER, {"shaders/one-color.fs.glsl"});
//        GLuint fs = utilCompileFiles(GL_FRAGMENT_SHADER, {"shaders/w-axis-hue.fs.glsl"});

//        pgm = utilLinkProgram({vs, fs});

        pgm = utilLinkProgram({
            utilCompileFiles(GL_VERTEX_SHADER, {"shaders/stereo-proper.vs.glsl"}),
//            utilCompileFiles(GL_VERTEX_SHADER, {"shaders/ortho.vs.glsl"}),
//            utilCompileFiles(GL_VERTEX_SHADER, {"shaders/stereo.vs.glsl"}),
            utilCompileFiles(GL_GEOMETRY_SHADER, {"shaders/stereo-proper.gm.glsl"}),
            utilCompileFiles(GL_FRAGMENT_SHADER, {"shaders/one-color.fs.glsl"}),
//            utilCompileFiles(GL_FRAGMENT_SHADER, {"shaders/w-axis-hue.fs.glsl"})
        });
    } catch (const gl_error &e) {
        std::cerr << e.what() << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    auto group = tc::group::H(4);
//    auto group = tc::group::B(5);
//    auto group = tc::group::A(5);
    GeomGen gg(group);
    auto res = gg.solve();
    auto mirrors = mirror(group);

    auto corners = plane_intersections(mirrors);
//    auto start = barycentric(corners, {.75f, .75f, .75f, .75f});
    auto start = barycentric(corners, {1.00f, 0.1f, 0.01f, 0.005f});
//    auto start = barycentric(corners, {1, 1, 0.05, 1});
    auto points = res.path.walk<glm::vec4, glm::vec4>(start, mirrors, reflect);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * points.size(), &points[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, false, 0, nullptr);

    std::vector<int> edge_count;
    std::vector<GLuint> edge_ibo;
    Simplexes base(1);
    base.vals = {0,1};
    std::vector<int> g_gens = gg.group_gens();

    printf("Num Edges:\n");
    for (const auto i : g_gens) {
        std::vector<int> sg_gens = {i};
        const auto data = gg.tile(g_gens, sg_gens, base).vals;
        edge_count.push_back(data.size());
        printf("\t%d: %d", i, data.size());

        GLuint ibo;
        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ARRAY_BUFFER, ibo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(int) * data.size(), &data[0], GL_STATIC_DRAW);
        edge_ibo.push_back(ibo);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    size_t numts = 7;
    float rates[numts];
    rates[0] = 1.237;
    rates[1] = 2.439;
    rates[2] = 5.683;
    rates[3] = -3.796;
    rates[4] = 0.787;
    rates[5] = -1.893;
    rates[6] = 3.699;

    float _ts[numts];
    float _ts_temp[numts];
    for (int i = 0; i < numts; ++i) {
        _ts[i] = ((float)i - (numts/2.f))*0.1f;
    }
    float* ts = _ts;
    float* ts_temp = _ts_temp;
    float* swap_t;
    float alpha = 0.0001;
    float beta = 0.0002;

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
//        glm::mat4 proj = glm::ortho(-pwidth, pwidth, -pheight, pheight, -2.0f, 2.0f);
        glm::mat4 proj = glm::ortho(-pwidth, pwidth, -pheight, pheight, -10.0f, 10.0f);
        glUniformMatrix4fv(0, 1, false, glm::value_ptr(proj));

        auto st = (float) glfwGetTime() / 8;
        auto t = st / 7;
//        printf("ts: [%f", ts[0]);
        ts_temp[0] = ts[0] + alpha * (float)std::cos(rates[0] * t) + beta * (-ts[0]);
        for (size_t i = 1; i < numts; i++){
//            printf( ", %f", ts[i]);
            ts_temp[i] = ts[i] + alpha * (float)std::cos(rates[i] * t) + beta * (-ts[i]);
        }
//        printf("]\n");
        swap_t = ts;
        ts = ts_temp;
        ts_temp = swap_t;
        auto view = glm::identity<glm::mat4>();
        view *= utilRotate(0, 1, st * ts[0]);
        view *= utilRotate(0, 2, st * ts[1]);
        view *= utilRotate(0, 3, st * ts[2]);
        view *= utilRotate(1, 2, st * ts[3]);
        view *= utilRotate(1, 3, st * ts[4]);
        view *= utilRotate(2, 3, st * ts[5]);
        glUniformMatrix4fv(1, 1, false, glm::value_ptr(view));
        //endregion

        glUniform3f(2, 1.0f, 1.0f, 1.0f);
        glDrawArrays(GL_POINTS, 0, points.size());

        glUniform3f(2, 1.0f, 1.0f, 1.0f);
        for (int i = 0; i < group.ngens; ++i) {
            auto ibo = edge_ibo[i];
            auto count = edge_count[i];

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
            glDrawElements(GL_LINES, count, GL_UNSIGNED_INT, nullptr);
        }

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}
