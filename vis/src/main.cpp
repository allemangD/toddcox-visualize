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
    if (state.dimension < 4) {
        view *= utilRotate(2, 3, M_PI_2f32);
    }

    if (state.dimension > 1) {
        view *= utilRotate(0, 1, state.st * .40f);
    }
    if (state.dimension > 2) {
        view *= utilRotate(0, 2, state.st * .20f);
        view *= utilRotate(1, 2, state.st * .50f);
    }
    if (state.dimension > 3) {
        view *= utilRotate(0, 3, state.st * 1.30f);
        view *= utilRotate(1, 3, state.st * .25f);
        view *= utilRotate(2, 3, state.st * 1.42f);
    }

    return Matrices(proj, view);
}

template<unsigned N, class T>
auto hull(const tc::Group &group, T all_sg_gens, const std::vector<std::vector<int>> &exclude) {
    std::vector<std::vector<Primitive<N>>> parts;
    auto g_gens = generators(group);
    for (const std::vector<int> &sg_gens : all_sg_gens) {
        bool excluded = false;
        for (const auto &test : exclude) {
            if (sg_gens == test) {
                excluded = true;
                break;
            }
        }
        if (excluded) continue;

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
struct Prop {
    cgl::VertexArray vao;
    cgl::Buffer<vec4> vbo;
    cgl::Buffer<Primitive<N>> ibo;

    Prop() : vao(), vbo(), ibo() {}
};

template<unsigned N>
struct Bundle : public std::vector<Prop<N>> {
};

template<unsigned N>
struct Renderer {
    virtual void bound(const std::function<void()> &action) const = 0;

    virtual void _draw(const Prop<N> &) const = 0;

    void draw(const Prop<N> &prop) const {
        bound([&]() {
            _draw(prop);
        });
    }

    void draw(const Bundle<N> &bundle) const {
        bound([&]() {
            for (const Prop<N> &prop : bundle) {
                _draw(prop);
            }
        });
    }
};

template<unsigned N>
struct Slice : public Prop<N> {
    vec3 color;

    Slice(vec3 color) : Prop<N>(), color(color) {}

    Slice(Slice &) = delete;

    Slice(Slice &&) noexcept = default;

    template<class T, class C>
    static Slice<N> build(
        const tc::Group &g,
        const C &coords,
        vec3 color,
        T all_sg_gens,
        const std::vector<std::vector<int>> &exclude
    ) {
        Slice<N> res(color);

        res.vbo.put(points(g, coords));
        res.ibo.put(merge<N>(hull<N>(g, all_sg_gens, exclude)));
        res.vao.ipointer(0, res.ibo, 4, GL_UNSIGNED_INT);

        return res;
    }
};

template<unsigned N>
struct SliceRenderer : public Renderer<N> {
    cgl::pgm::vert defer = cgl::pgm::vert::file(
        "shaders/slice/deferred.vs.glsl");
    cgl::pgm::geom slice = cgl::pgm::geom::file(
        "shaders/slice/slice.gm.glsl");
    cgl::pgm::frag solid = cgl::pgm::frag::file(
        "shaders/solid.fs.glsl");

    cgl::pipeline pipe;

    SliceRenderer() {
        pipe.stage(defer);
        pipe.stage(slice);
        pipe.stage(solid);
    }

    SliceRenderer(SliceRenderer &) = delete;

    SliceRenderer(SliceRenderer &&) noexcept = default;

    void bound(const std::function<void()> &action) const override {
        pipe.bound(action);
    }

    void _draw(const Prop<N> &prop) const override {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, prop.vbo);
//        glProgramUniform3fv(solid, 2, 1, &prop.color.front());
        glProgramUniform3f(solid, 2, 1.f, 1.f, 1.f);
        prop.vao.bound([&]() {
            glDrawArrays(GL_POINTS, 0, prop.ibo.count() * N);
        });
    }
};


template<unsigned N>
struct DirectRenderer : public Renderer<N> {
    cgl::pgm::vert direct_stereo = cgl::pgm::vert::file(
        "shaders/direct-stereo.vs.glsl");
    cgl::pgm::frag solid = cgl::pgm::frag::file(
        "shaders/solid.fs.glsl");

    cgl::pipeline pipe;

    DirectRenderer() {
        pipe.stage(direct_stereo);
        pipe.stage(solid);
    }

    DirectRenderer(DirectRenderer &) = delete;

    DirectRenderer(DirectRenderer &&) noexcept = default;

    void bound(const std::function<void()> &action) const override {
        pipe.bound(action);
    }

    void _draw(const Prop<N> &prop) const override {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, prop.vbo);
//        glProgramUniform3fv(sh.solid, 2, 1, &wire.color.front());
        glProgramUniform3f(solid, 2, .3f, .3f, .3f);
        prop.vao.bound([&]() {
            prop.ibo.bound(GL_ELEMENT_ARRAY_BUFFER, [&]() {
                glDrawElements(GL_LINES, prop.ibo.count() * N, GL_UNSIGNED_INT, nullptr);
            });
        });
    }
};

struct Wire : public Prop<2> {
    vec3 color;

    Wire(vec3 color) : Prop<2>(), color(color) {}

    Wire(Wire &) = delete;

    Wire(Wire &&) noexcept = default;

    template<class T, class C>
    static Wire build(const tc::Group &g,
        const C &coords,
        bool curve,
        bool ortho,
        vec3 color,
        T all_sg_gens,
        const std::vector<std::vector<int>> &exclude
    ) {
        Wire res(color);

        res.vbo.put(points(g, coords));
        res.ibo.put(merge<2>(hull<2>(g, all_sg_gens, exclude)));

        return res;
    }
};

void run(const std::string &config_file, GLFWwindow *window) {
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    SliceRenderer<4> sRen{};
    DirectRenderer<2> wRen{};

    auto scene = YAML::LoadFile(config_file);

    State state{};
    glfwSetWindowUserPointer(window, &state);

    state.dimension = scene["dimension"].as<int>();

    auto slices = Bundle<4>();
    auto wires = Bundle<2>();

    for (const auto &group_info : scene["groups"]) {
        auto symbol = group_info["symbol"].as<std::vector<int>>();
        auto group = tc::schlafli(symbol);
        auto gens = generators(group);

        if (group_info["slices"].IsDefined()) {
            for (const auto &slice_info : group_info["slices"]) {
                auto root = slice_info["root"].as<vec5>();
                auto color = slice_info["color"].as<vec3>();
                auto exclude = std::vector<std::vector<int>>();

                if (slice_info["exclude"].IsDefined()) {
                    exclude = slice_info["exclude"].as<std::vector<std::vector<int>>>();
                }

                if (slice_info["subgroups"].IsDefined()) {
                    auto subgroups = slice_info["subgroups"].as<std::vector<std::vector<int>>>();
                    slices.push_back(Slice<4>::build(
                        group, root, color, subgroups, exclude
                    ));
                } else {
                    auto combos = Combos<int>(gens, 3);
                    slices.push_back(Slice<4>::build(
                        group, root, color, combos, exclude
                    ));
                }
            }
        }

        if (group_info["wires"].IsDefined()) {
            for (const auto &wire_info : group_info["wires"]) {
                auto root = wire_info["root"].as<vec5>();
                auto color = wire_info["color"].as<vec3>();
                auto exclude = std::vector<std::vector<int>>();
                auto curve = wire_info["curve"].IsDefined() && wire_info["curve"].as<bool>();
                auto ortho = wire_info["ortho"].IsDefined() && wire_info["ortho"].as<bool>();

                if (wire_info["exclude"].IsDefined()) {
                    exclude = wire_info["exclude"].as<std::vector<std::vector<int>>>();
                }

                if (wire_info["subgroups"].IsDefined()) {
                    auto subgroups = wire_info["subgroups"].as<std::vector<std::vector<int>>>();
                    wires.push_back(Wire::build(
                        group, root, curve, ortho, color, subgroups, exclude
                    ));
                } else {
                    auto combos = Combos<int>(gens, 1);
                    wires.push_back(Wire::build(
                        group, root, curve, ortho, color, combos, exclude
                    ));
                }
            }
        }
    }

    auto ubo = cgl::Buffer<Matrices>();
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo);

    while (!glfwWindowShouldClose(window)) {
        auto time = (float) glfwGetTime();
        state.time_delta = state.time - time;
        state.time = time;

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Matrices mats = build(window, state);
        ubo.put(mats);

        glLineWidth(1.5);

        wRen.draw(wires);
        sRen.draw(slices);

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
