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

template<unsigned N, class V>
struct Drawable {
    GLenum mode{};
    cgl::VertexArray vao{};
    cgl::Buffer<Primitive<N>> ibo{};
    cgl::Buffer<V> vbo{};

    Drawable(GLenum mode) : mode(mode), vao(), ibo(), vbo() {}

    Drawable(Drawable &) = delete;

    Drawable(Drawable &&) = delete;

    void draw_deferred() {
        vao.bound([&]() {
            glDrawArrays(GL_POINTS, 0, ibo.count() * N);
        });
    }

    void draw_direct() {
        vao.bound([&]() {
            ibo.bound(GL_ELEMENT_ARRAY_BUFFER, [&]() {
                glDrawElements(mode, ibo.count() * N, GL_UNSIGNED_INT, nullptr);
            });
        });
    }
};

float factor(unsigned index, unsigned size) {
    auto num = (float) index;
    auto den = size > 1 ? (float) size - 1 : 1;
    return num / den;
}

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

template<unsigned N>
std::vector<std::vector<Primitive<N>>> poly_parts(const tc::Group &group) {
    std::vector<std::vector<Primitive<N>>> parts;
    auto g_gens = gens(group);
    for (const auto &sg_gens : Combos(g_gens, N - 1)) {
        const auto &base = triangulate<N>(group, sg_gens);
        const auto &all = tile(base, group, g_gens, sg_gens);
        parts.push_back(all);
    }
    return parts;
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

std::vector<vec4> points(const tc::Group &group) {
    auto cosets = group.solve();
    auto mirrors = mirror<5>(group);

    auto corners = plane_intersections(mirrors);

    auto start = barycentric(corners, {1.0f, 0.2f, 0.1f, 0.05f, 0.025f});
//    auto start = barycentric(corners, {1.0f, 1.0f, 1.0f, 1.0f});
//    auto start = barycentric(corners, {0.05f, 0.1f, 0.2f, 1.00f});

    auto higher = cosets.path.walk<vec5, vec5>(start, mirrors, reflect<vec5>);
    std::vector<vec4> res(higher.size());
    std::transform(higher.begin(), higher.end(), res.begin(), stereo<4>);

    return res;
}

void run(GLFWwindow *window) {
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
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
        .stage(sh.diffuse);

    auto group = tc::schlafli({5, 3, 3, 2});

    auto wire_data = merge(poly_parts<2>(group));

    const auto slice_light = glm::vec3(.9, .9, .95);
//    const auto slice_dark = slice_light;
    const auto slice_dark = glm::vec3(.5, .3, .7);

    auto slice_parts = poly_parts<4>(group);
    slice_parts.erase(slice_parts.end() - 1, slice_parts.end());
    auto slice_data = merge(slice_parts);

    auto slice_colors = std::vector<glm::vec3>(slice_data.size());
    for (int i = 0, k = 0; i < slice_parts.size(); ++i) {
        auto fac = factor(i, slice_parts.size());
        glm::vec3 color = glm::mix(slice_dark, slice_light, fac);

        for (int j = 0; j < slice_parts[i].size(); ++j, ++k) {
            slice_colors[k] = color;
        }
    }

    Drawable<2, float> wires(GL_LINES);
    wires.ibo.put(wire_data);

    Drawable<4, glm::vec3> slices(GL_POINTS);
    slices.ibo.put(slice_data);
    slices.vbo.put(slice_colors);
    slices.vao.ipointer(0, slices.ibo, 4, GL_UNSIGNED_INT);
    slices.vao.pointer(1, slices.vbo, 3, GL_FLOAT);

    auto vbo = cgl::Buffer<vec4>(points(group));
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vbo);

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
            slices.draw_deferred();
        });

//        glProgramUniform4f(sh.solid, 2, 0.3, 0.3, 0.3, 0.4);
//        proj_pipe.bound([&]() {
//            wires.draw_direct();
//        });

        glfwSwapInterval(2);
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
