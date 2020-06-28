#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>

#include <tc/groups.hpp>

#include "util.hpp"
#include "mirror.hpp"
#include "geometry.hpp"

#include <cgl/vertexarray.hpp>
#include <cgl/shaderprogram.hpp>
#include <cgl/pipeline.hpp>
#include <random>

#include <chrono>
#include <yaml-cpp/yaml.h>

#include <rendering.hpp>

#ifdef _WIN32
extern "C" {
__attribute__((unused)) __declspec(dllexport) int NvOptimusEnablement = 0x00000001;
}
#endif

struct Matrices {
    glm::mat4 proj;
    glm::mat4 view;

    Matrices() = default;

    Matrices(const glm::mat4 &proj, const glm::mat4 &view)
        : proj(proj), view(view) {
    }
};

struct State {
    float time;
    float time_delta;

    float st;

    int dimension;
};

Matrices build(GLFWwindow *window, State &state) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    auto aspect = (float) width / (float) height;
    auto pheight = 1.4f;
    auto pwidth = aspect * pheight;
    glm::mat4 proj = glm::ortho(-pwidth, pwidth, -pheight, pheight, -10.0f, 10.0f);

    if (!glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
        state.st += state.time_delta / 8;
    }

    auto view = glm::identity<glm::mat4>();
    return Matrices(proj, view);
}

template<class C>
std::vector<vec4> points(const tc::Group &group, const C &coords, const float time) {
    auto cosets = group.solve();
    auto mirrors = mirror<5>(group);

    auto corners = plane_intersections(mirrors);
    auto start = barycentric(corners, coords);

    auto higher = cosets.path.walk<vec5, vec5>(start, mirrors, reflect<vec5>);

    auto r = identity<5>();
    r = mul(r, rot<5>(0, 2, time * .21f));
    r = mul(r, rot<5>(1, 4, time * .27f));

    r = mul(r, rot<5>(0, 3, time * .17f));
    r = mul(r, rot<5>(1, 3, time * .25f));
    r = mul(r, rot<5>(2, 3, time * .12f));

    std::transform(higher.begin(), higher.end(), higher.begin(), [&](vec5 v) { return mul(v, r); });

    std::vector<vec4> lower(higher.size());
    std::transform(higher.begin(), higher.end(), lower.begin(), stereo<4>);
    return lower;
}

template<int N, class T, class C>
Prop<4, vec4> make_slice(
    const tc::Group &g,
    const C &coords,
    vec3 color,
    T all_sg_gens,
    const std::vector<std::vector<int>> &exclude
) {
    Prop<N, vec4> res{};

//    res.vbo.put(points(g, coords));
    res.ibo.put(merge<N>(hull<N>(g, all_sg_gens, exclude)));
    res.vao.ipointer(0, res.ibo, 4, GL_UNSIGNED_INT);

    return res;
}

void run(const std::string &config_file, GLFWwindow *window) {
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::vector<int> symbol = {4, 3, 3, 3};
    vec5 root = {.80, .02, .02, .02, .02};

    auto group = tc::schlafli(symbol);
    auto gens = generators(group);
    std::vector<std::vector<int>> exclude = {{0, 1, 2}};
    auto combos = Combos<int>(gens, 3);

    SliceRenderer<4, vec4> ren{};
    Prop<4, vec4> prop = make_slice<4>(group, root, {}, combos, exclude);

    State state{};
    state.dimension = 4;
    glfwSetWindowUserPointer(window, &state);

    auto ubo = cgl::Buffer<Matrices>();
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo);

    while (!glfwWindowShouldClose(window)) {
        auto time = (float) glfwGetTime();
        state.time_delta = state.time - time;
        state.time = time;

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLineWidth(1.5);

        prop.vbo.put(points(group, root, time));

        Matrices mats{};

        glViewport(0, 0, width, height);
        mats = build(window, state);
        ubo.put(mats);
        ren.draw(prop);

        glfwSwapInterval(2);
        glfwSwapBuffers(window);

        glfwPollEvents();
    }
}

int main(int argc, char *argv[]) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_VERSION_MAJOR, 5);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

    std::cout << utilInfo();

    std::string config_file = "presets/default.yaml";
    if (argc > 1) config_file = std::string(argv[1]);

    run(config_file, window);

    glfwTerminate();
    return EXIT_SUCCESS;
}
