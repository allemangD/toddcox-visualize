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

#ifdef _WIN32
extern "C" {
__attribute__((unused)) __declspec(dllexport) int NvOptimusEnablement = 0x00000001;
}
#endif

struct Matrices {
    glm::mat4 proj;
    glm::mat4 view;

    Matrices(const glm::mat4 &proj, const glm::mat4 &view)
        : proj(proj), view(view) {
    }
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

    return Matrices(proj, view);
}

template<unsigned N, class T>
auto hull(const tc::Group &group, T begin, T end) {
    std::vector<std::vector<Primitive<N>>> parts;
    auto g_gens = gens(group);
    while (begin != end) {
        const auto &sg_gens = *(++begin);
        const auto &base = triangulate<N>(group, sg_gens);
        const auto &tiles = each_tile(base, group, g_gens, sg_gens);
        for (const auto &tile : tiles) {
            parts.push_back(tile);
        }
    }
    return parts;
}

template<class C>
std::vector<vec4> points(const tc::Group &group, const C &coords) {
    auto cosets = group.solve();
    auto mirrors = mirror<5>(group);

    auto corners = plane_intersections(mirrors);

    auto start = barycentric(corners, coords);

    const auto &higher = cosets.path.walk<vec5, vec5>(start, mirrors, reflect<vec5>);
    std::vector<vec4> lower(higher.size());
    std::transform(higher.begin(), higher.end(), lower.begin(), stereo<4>);
    return lower;
}

class Shaders {
public:
    cgl::pgm::vert defer = cgl::pgm::vert::file(
        "shaders/slice/deferred.vs.glsl");
    cgl::pgm::vert direct_ortho = cgl::pgm::vert::file(
        "shaders/direct-ortho.vs.glsl");
    cgl::pgm::vert direct_stereo = cgl::pgm::vert::file(
        "shaders/direct-stereo.vs.glsl");

    cgl::pgm::geom slice = cgl::pgm::geom::file(
        "shaders/slice/slice.gm.glsl");
    cgl::pgm::geom curve_stereo = cgl::pgm::geom::file(
        "shaders/curve-stereo.gm.glsl");
    cgl::pgm::geom curve_ortho = cgl::pgm::geom::file(
        "shaders/curve-ortho.gm.glsl");

    cgl::pgm::frag solid = cgl::pgm::frag::file(
        "shaders/solid.fs.glsl");
    cgl::pgm::frag diffuse = cgl::pgm::frag::file(
        "shaders/diffuse.fs.glsl");
};

template<unsigned N>
struct Slice {
    GLenum mode;
    vec4 color;
    cgl::VertexArray vao;
    cgl::Buffer<vec4> vbo;
    cgl::Buffer<Primitive<N>> ibo;

    Slice(GLenum mode, vec4 color) : mode(mode), color(color), vao(), ibo(), vbo() {}

    Slice(Slice &) = delete;

    Slice(Slice &&) noexcept = default;

    void draw() const {
        vao.bound([&]() {
            glDrawArrays(GL_POINTS, 0, ibo.count() * N);
        });
    }

    template<class T, class C>
    static Slice<N> build(const tc::Group &g, const C &coords, vec4 color, T begin, T end) {
        Slice<N> res(GL_POINTS, color);

        res.vbo.put(points(g, coords));
        res.ibo.put(merge<N>(hull<N>(g, begin, end)));
        res.vao.ipointer(0, res.ibo, 4, GL_UNSIGNED_INT);

        return res;
    }
};

void run(GLFWwindow *window) {
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shaders sh;

    auto proj_pipe = cgl::pipeline();
    proj_pipe
        .stage(sh.direct_stereo)
        .stage(sh.solid);

    auto slice_pipe = cgl::pipeline();
    slice_pipe
        .stage(sh.defer)
        .stage(sh.slice)
        .stage(sh.solid);

    auto group = tc::schlafli({3, 4, 3, 2});

    const auto combos = Combos<int>({0, 1, 2, 3, 4}, 3);

    auto slices = {
        Slice<4>::build(
            group,
            (vec5) {1.0, 0.1, 0.1, 0.1, 0.025} * 0.8,
            {0.9f, 0.9f, 0.9f, 1.0f},
            combos.begin(), combos.end()),
        Slice<4>::build(
            group,
            (vec5) {1.0, 0.1, 0.1, 0.1, 0.025},
            {0.3f, 0.3f, 0.3f, 1.0f},
            combos.begin()++, combos.end()),
    };

    auto ubo = cgl::Buffer<Matrices>();
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo);

    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto st = (float) glfwGetTime() / 8;
        Matrices mats = build(window, st);
        ubo.put(mats);

        glLineWidth(1.5);

        slice_pipe.bound([&]() {
            for (const auto &slice : slices) {
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, slice.vbo);
                glProgramUniform4fv(sh.solid, 2, 1, &slice.color.front());
                slice.draw();
            }
        });

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

    run(window);

    glfwTerminate();
    return EXIT_SUCCESS;
}
