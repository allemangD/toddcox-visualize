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


template<int D>
tc::Group shrink(const tc::Group &g, const std::array<int, D> &sub) {
    tc::Group h(D);
    for (int i = 0; i < D; ++i) {
        for (int j = 0; j < D; ++j) {
            h.setmult({i, j, g.rel(sub[i], sub[j]).mult});
        }
    }
    return h;
}

template<int D>
std::vector<tc::Action> raise(
    const tc::Cosets &cosets,
    const std::vector<tc::Action> &path,
    const std::array<int, D> &gen_map
) {
    std::vector<tc::Action> res(path.size(), {0, 0, 0});
    for (size_t i = 1; i < path.size(); ++i) {
        auto action = path[i];

        auto coset = res[action.coset].target;
        auto gen = gen_map[action.gen];
        auto target = cosets.get(coset, gen);

        res[i] = {coset, gen, target};
    }
    return res;
}

std::vector<int> targets(const std::vector<tc::Action> &path) {
    std::vector<int> res(path.size(), 0);
    for (size_t i = 0; i < path.size(); ++i) {
        res[i] = path[i].target;
    }
    return res;
}

std::vector<int> tile(const tc::Cosets &map, std::vector<int> geom) {
    int K = geom.size();
    geom.resize(K * map.size());
    for (int i = 1; i < map.size(); ++i) { // tile
        auto gaction = map.path[i];
        for (int j = 0; j < K; ++j) {
            geom[i * K + j] = map.get(geom[gaction.coset * K + j], gaction.gen);
        }
    }
    return geom;
}

template<int D>
std::vector<int> build(const tc::Group &g, const std::array<int, D> &sub) {
    tc::Group h = shrink<D>(g, sub);
    auto hres = tc::solve(h); // recursion would happen here

    auto gres = tc::solve(g);
    auto path = raise<D>(gres, hres.path, sub);

    const std::vector<int> &geom = targets(path);

    return tile(gres, geom);
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

    auto group = tc::group::H(4);
    auto res = tc::solve(group);
    auto mirrors = mirror(group);
    auto corners = plane_intersections(mirrors);

    auto points = std::vector<glm::vec4>(res.size());
    points[0] = barycentric(corners, {1.00f, 0.50f, 0.50f, 0.50f});
    for (int i = 1; i < res.size(); ++i) {
        auto action = res.path[i];
        points[i] = reflect(points[action.coset], mirrors[action.gen]);
    }

    auto g0 = build<2>(group, {0});
    auto g1 = build<2>(group, {1});
    auto g2 = build<2>(group, {2});
    auto g3 = build<2>(group, {3});

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    utilShaderSource(vs, {
        "#version 430\n",

        "layout(location=0) uniform mat4 proj;"
        "layout(location=1) uniform mat4 view;"
        ""
        "layout(location=0) in vec4 pos;"
        ""
        "out vec4 vpos;"
        ""
        "void main() {"
        "   int i = gl_VertexID;"
        "   vpos = view * pos;"
        "   gl_Position = proj * vec4(vpos.xyz / (1 - vpos.w), 1);"
        "   gl_PointSize = 5;"
        "}"
    });
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    utilShaderSource(fs, {
        "#version 430\n",

        "layout(location=2) uniform vec3 c;"
        ""
        "in vec4 vpos;"
        ""
        "out vec4 color;"
        ""
        "void main() {"
        "   float d = smoothstep(-2, 2, vpos.z);"
        "   color = vec4(c * d, 1);"
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

    GLuint ibo0;
    glGenBuffers(1, &ibo0);
    glBindBuffer(GL_ARRAY_BUFFER, ibo0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(int) * g0.size(), &g0[0], GL_STATIC_DRAW);

    GLuint ibo1;
    glGenBuffers(1, &ibo1);
    glBindBuffer(GL_ARRAY_BUFFER, ibo1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(int) * g1.size(), &g1[0], GL_STATIC_DRAW);

    GLuint ibo2;
    glGenBuffers(1, &ibo2);
    glBindBuffer(GL_ARRAY_BUFFER, ibo2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(int) * g2.size(), &g2[0], GL_STATIC_DRAW);

    GLuint ibo3;
    glGenBuffers(1, &ibo3);
    glBindBuffer(GL_ARRAY_BUFFER, ibo3);
    glBufferData(GL_ARRAY_BUFFER, sizeof(int) * g3.size(), &g3[0], GL_STATIC_DRAW);

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
        auto view = glm::identity<glm::mat4>();
        view = glm::rotate(view, t / 1, glm::vec3(0, 1, 0));
        view = glm::rotate(view, t / 3, glm::vec3(0, 0, 1));
        view = glm::rotate(view, t / 4, glm::vec3(1, 0, 0));
        glUniformMatrix4fv(1, 1, false, glm::value_ptr(view));

        glUniform3f(2, 1.0f, 1.0f, 1.0f);
        glDrawArrays(GL_POINTS, 0, points.size());

        glLineWidth(2.0f);
        glUniform3f(2, 1.0f, 0.0f, 0.0f);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo0);
        glDrawElements(GL_LINES, g0.size(), GL_UNSIGNED_INT, nullptr);

        glUniform3f(2, 0.0f, 1.0f, 0.0f);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo1);
        glDrawElements(GL_LINES, g1.size(), GL_UNSIGNED_INT, nullptr);

        glUniform3f(2, 0.0f, 0.0f, 1.0f);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo2);
        glDrawElements(GL_LINES, g2.size(), GL_UNSIGNED_INT, nullptr);

        glUniform3f(2, 0.5f, 0.5f, 0.0f);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo3);
        glDrawElements(GL_LINES, g3.size(), GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}
