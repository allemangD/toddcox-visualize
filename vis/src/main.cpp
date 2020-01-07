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

std::vector<int> build(const tc::Group &g, std::vector<int> gens) {
    std::cout << g.name << std::endl;

    if (g.trivial()) return {0};

    std::vector<int> res;

    auto root = tc::solve(g);

    for (size_t i = 0; i < gens.size(); ++i) {
        std::vector<int> sub(gens);
        sub.erase(sub.begin() + i);

        auto base = build(g.shrink(sub), sub);
        for (auto e : base) {
            res.push_back(e);
        }
        res.push_back(0);
    }

    auto map = tc::solve(g, gens);
    size_t N = res.size();
    res.resize(N * map.size());

    for (size_t i = 1; i < res.size(); ++i) {
        auto action = map.path[i];
        for (size_t j = 0; j < N; ++j) {
            res[i * N + j] = root.get(res[action.coset * N + j], action.gen);
        }
    }

    return res;
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

    auto group = tc::group::B(3);
    auto res = tc::solve(group);
    auto mirrors = mirror(group);
    auto corners = plane_intersections(mirrors);

    auto points = std::vector<glm::vec4>(res.size());
    points[0] = barycentric(corners, {1.00f, 0.50f, 0.50f, 0.50f});
    for (int i = 1; i < res.size(); ++i) {
        auto action = res.path[i];
        points[i] = reflect(points[action.coset], mirrors[action.gen]);
    }

    auto res0 = tc::solve(group, {0});
    auto res0_inds = std::vector<int>(res0.size() * 2);
    res0_inds[0] = res.get(0, 0);
    res0_inds[1] = 0;
    for (int i = 1; i < res0.size(); ++i) {
        auto action = res0.path[i];
        res0_inds[i * 2 + 0] = res.get(res0_inds[action.coset * 2 + 0], action.gen);
        res0_inds[i * 2 + 1] = res.get(res0_inds[action.coset * 2 + 1], action.gen);
    }

    auto res1 = tc::solve(group, {1});
    auto res1_inds = std::vector<int>(res1.size() * 2);
    res1_inds[0] = res.get(0, 1);
    res1_inds[1] = 0;
    for (int i = 1; i < res1.size(); ++i) {
        auto action = res1.path[i];
        res1_inds[i * 2 + 0] = res.get(res1_inds[action.coset * 2 + 0], action.gen);
        res1_inds[i * 2 + 1] = res.get(res1_inds[action.coset * 2 + 1], action.gen);
    }

    auto res2 = tc::solve(group, {2});
    auto res2_inds = std::vector<int>(res2.size() * 2);
    res2_inds[0] = res.get(0, 2);
    res2_inds[1] = 0;
    for (int i = 1; i < res2.size(); ++i) {
        auto action = res2.path[i];
        res2_inds[i * 2 + 0] = res.get(res2_inds[action.coset * 2 + 0], action.gen);
        res2_inds[i * 2 + 1] = res.get(res2_inds[action.coset * 2 + 1], action.gen);
    }

//    auto res01 = tc::solve(group, {0, 1});
//    auto res01_inds = std::vector<int>(res01.size() * 3);
//    res01_inds[0] = res.get(0, 0);
//    res01_inds[1] = res.get(0, 1);
//    res01_inds[2] = 0;
//    for (int i = 0; i < res01.size(); ++i) {
//        auto action = res01.path[i];
//        for (int j = 0; j < 3; ++j) {
//            res01_inds[i * 3 + j] = res.get(res01_inds[action.coset * 3 + j], action.gen);
//        }
//    }

    auto test_inds = build(group, {0, 1, 2});
    std::cout << "done" << std::endl;

    auto test_mode = GL_TRIANGLES;

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    utilShaderSource(vs, {
        "#version 430\n",

        "layout(location=0) uniform mat4 proj;"
        "layout(location=1) uniform mat4 view;"
        ""
        "layout(location=0) in vec4 pos;"
        ""
        "void main() {"
        "   int i = gl_VertexID;"
        "   gl_Position = proj * view * vec4(pos.xyz, 1);"
        "   gl_PointSize = 5;"
        "}"
    });
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    utilShaderSource(fs, {
        "#version 430\n",

        "layout(location=2) uniform float gray;"
        ""
        "out vec4 color;"
        ""
        "void main() {"
        "   color = vec4(gray);"
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

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * points.size(), &points[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, false, 0, nullptr);

    GLuint res0_ibo;
    glGenBuffers(1, &res0_ibo);
    glBindBuffer(GL_ARRAY_BUFFER, res0_ibo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(int) * res0_inds.size(), &res0_inds[0], GL_STATIC_DRAW);

    GLuint res1_ibo;
    glGenBuffers(1, &res1_ibo);
    glBindBuffer(GL_ARRAY_BUFFER, res1_ibo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(int) * res1_inds.size(), &res1_inds[0], GL_STATIC_DRAW);

    GLuint res2_ibo;
    glGenBuffers(1, &res2_ibo);
    glBindBuffer(GL_ARRAY_BUFFER, res2_ibo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(int) * res2_inds.size(), &res2_inds[0], GL_STATIC_DRAW);

    GLuint test_ibo;
    glGenBuffers(1, &test_ibo);
    glBindBuffer(GL_ARRAY_BUFFER, test_ibo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(int) * test_inds.size(), &test_inds[0], GL_STATIC_DRAW);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(pgm);
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_POINT_SMOOTH);
        glEnable(GL_DEPTH_TEST);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        auto aspect = (float) width / (float) height;
        auto pheight = 1.4f;
        auto pwidth = aspect * pheight;
        glm::mat4 proj = glm::ortho(-pwidth, pwidth, -pheight, pheight);
        glUniformMatrix4fv(0, 1, false, glm::value_ptr(proj));

        auto t = (float) glfwGetTime() / 3;
        glm::mat4 view = glm::rotate(glm::identity<glm::mat4>(), t, glm::vec3(0, 1, 0));
        glUniformMatrix4fv(1, 1, false, glm::value_ptr(view));

        glUniform1f(2, 1.0f);
        glDrawArrays(GL_POINTS, 0, points.size());

//        glUniform1f(2, 0.9f);
//        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, res0_ibo);
//        glDrawElements(GL_LINES, res0_inds.size(), GL_UNSIGNED_INT, nullptr);
//        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, res1_ibo);
//        glDrawElements(GL_LINES, res1_inds.size(), GL_UNSIGNED_INT, nullptr);
//        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, res2_ibo);
//        glDrawElements(GL_LINES, res2_inds.size(), GL_UNSIGNED_INT, nullptr);

        glUniform1f(2, 0.5f);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, test_ibo);
        glDrawElements(test_mode, test_inds.size(), GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}
