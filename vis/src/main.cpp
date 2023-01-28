#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>

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

#include <shaders.hpp>

#ifdef _WIN32
extern "C" {
__attribute__((unused)) __declspec(dllexport) int NvOptimusEnablement = 0x00000001;
}
#endif

struct Matrices {
    Eigen::Matrix4f proj;
    Eigen::Matrix4f view;

    Matrices(const Eigen::Matrix4f &proj, const Eigen::Matrix4f &view)
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
    Eigen::Matrix4f proj = ortho(-pwidth, pwidth, -pheight, pheight, -10.0f, 10.0f);

    if (!glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
        state.st += state.time_delta / 8;
    }

    Eigen::Matrix4f view;
    view.setIdentity();

    if (state.dimension < 4) {
        view *= utilRotate(2, 3, M_PI_2f32 + 0.01f);
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

template<class C>
std::vector<vec4> points(const tc::Group<> &group, const C &coords) {
    auto cosets = group.solve();
    auto mirrors = mirror<5>(group);

    tc::Path<vec5> path(cosets, mirrors);

    auto corners = plane_intersections(mirrors);

    auto start = barycentric(corners, coords);

    std::vector<vec5> higher(path.order());
    path.walk(start, reflect<vec5>, higher.begin());

    std::vector<vec4> lower(higher.size());
    std::transform(higher.begin(), higher.end(), lower.begin(), stereo<4>);

    return lower;
}

template<unsigned N>
struct Prop {
    cgl::VertexArray vao;
    cgl::Buffer<vec4> vbo;
    cgl::Buffer<Primitive<N>> ibo;

    vec3 color;

    Prop() : vao(), vbo(), ibo() {}
};

template<unsigned N>
struct Renderer {
    std::vector<Prop<N>> props;

    virtual void bound(const std::function<void()> &action) const = 0;

    virtual void _draw(const Prop<N> &) const = 0;

    void render() const {
        bound([&]() {
            for (const auto &prop : props) {
                _draw(prop);
            }
        });
    }
};

template<unsigned N>
struct SliceProp : public Prop<N> {
    SliceProp(vec3 color) : Prop<N>() {
        this->color = color;
    }

    SliceProp(SliceProp &) = delete;

    SliceProp(SliceProp &&) noexcept = default;

    template<class T, class C>
    static SliceProp<N> build(
        const tc::Group<> &g,
        const C &coords,
        vec3 color,
        T all_sg_gens,
        const std::vector<std::vector<size_t>> &exclude
    ) {
        SliceProp<N> res(color);

        res.vbo.put(points(g, coords));
        res.ibo.put(merge<N>(hull<N>(g, all_sg_gens, exclude)));
        res.vao.ipointer(0, res.ibo, 4, GL_UNSIGNED_INT);

        return res;
    }
};

template<unsigned N>
struct SliceRenderer : public Renderer<N> {
    cgl::pgm::vert defer = cgl::pgm::vert(shaders::deferred_vs_glsl);
    cgl::pgm::geom slice = cgl::pgm::geom(shaders::slice_gm_glsl);
    cgl::pgm::frag solid = cgl::pgm::frag(shaders::solid_fs_glsl);

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
        glProgramUniform3fv(solid, 2, 1, &prop.color.front());
//        glProgramUniform3f(solid, 2, 1.f, 1.f, 1.f);
        prop.vao.bound([&]() {
            glDrawArrays(GL_POINTS, 0, prop.ibo.count() * N);
        });
    }
};


template<unsigned N>
struct DirectRenderer : public Renderer<N> {
    cgl::pipeline pipe;

    cgl::pgm::frag solid = cgl::pgm::frag(shaders::solid_fs_glsl);

    DirectRenderer() {
        pipe.stage(solid);
    };

    DirectRenderer(DirectRenderer &) = delete;

    DirectRenderer(DirectRenderer &&) noexcept = default;

    void bound(const std::function<void()> &action) const override {
        pipe.bound(action);
    }

    void _draw(const Prop<N> &prop) const override {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, prop.vbo);
        glProgramUniform3fv(solid, 2, 1, &prop.color.front());
        prop.vao.bound([&]() {
            prop.ibo.bound(GL_ELEMENT_ARRAY_BUFFER, [&]() {
                glDrawElements(GL_LINES, prop.ibo.count() * N, GL_UNSIGNED_INT, nullptr);
            });
        });
    }
};

struct WireframeProp : public Prop<2> {

    WireframeProp(vec3 color) : Prop<2>(){
        this->color = color;
    }

    WireframeProp(WireframeProp &) = delete;

    WireframeProp(WireframeProp &&) noexcept = default;

    template<class T, class C>
    static WireframeProp build(const tc::Group<> &g,
        const C &coords,
        bool curve,
        bool ortho,
        vec3 color,
        T all_sg_gens,
        const std::vector<std::vector<size_t>> &exclude
    ) {
        WireframeProp res(color);

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

    cgl::pgm::vert o = cgl::pgm::vert(shaders::direct_ortho_vs_glsl);
    cgl::pgm::vert s = cgl::pgm::vert(shaders::direct_stereo_vs_glsl);
    cgl::pgm::geom co = cgl::pgm::geom(shaders::curve_ortho_gm_glsl);
    cgl::pgm::geom cs = cgl::pgm::geom(shaders::curve_stereo_gm_glsl);

    DirectRenderer<2> woRen{};
    woRen.pipe.stage(o);

    DirectRenderer<2> wocRen{};
    wocRen.pipe.stage(o);
    wocRen.pipe.stage(co);

    DirectRenderer<2> wsRen{};
    wsRen.pipe.stage(s);

    DirectRenderer<2> wscRen{};
    wscRen.pipe.stage(s);
    wscRen.pipe.stage(cs);

    auto scene = YAML::LoadFile(config_file);

    State state{};
    glfwSetWindowUserPointer(window, &state);

    state.dimension = scene["dimension"].as<size_t>();

    for (const auto &group_info : scene["groups"]) {
        auto symbol = group_info["symbol"].as<std::vector<unsigned int>>();
        auto group = tc::schlafli(symbol);
        auto gens = generators(group);

        if (group_info["slices"].IsDefined()) {
            for (const auto &slice_info : group_info["slices"]) {
                auto root = slice_info["root"].as<vec5>();
                auto color = slice_info["color"].as<vec3>();
                auto exclude = std::vector<std::vector<size_t>>();

                if (slice_info["exclude"].IsDefined()) {
                    exclude = slice_info["exclude"].as<std::vector<std::vector<size_t>>>();
                }

                if (slice_info["subgroups"].IsDefined()) {
                    auto subgroups = slice_info["subgroups"].as<std::vector<std::vector<size_t>>>();
                    sRen.props.push_back(SliceProp<4>::build(
                        group, root, color, subgroups, exclude
                    ));
                } else {
                    auto combos = Combos<size_t>(gens, 3);
                    sRen.props.push_back(SliceProp<4>::build(
                        group, root, color, combos, exclude
                    ));
                }
            }
        }

        if (group_info["wires"].IsDefined()) {
            for (const auto &wire_info : group_info["wires"]) {
                auto root = wire_info["root"].as<vec5>();
                auto color = wire_info["color"].as<vec3>();
                auto exclude = std::vector<std::vector<size_t>>();
                auto curve = wire_info["curve"].IsDefined() && wire_info["curve"].as<bool>();
                auto ortho = wire_info["ortho"].IsDefined() && wire_info["ortho"].as<bool>();

                if (wire_info["exclude"].IsDefined()) {
                    exclude = wire_info["exclude"].as<std::vector<std::vector<size_t>>>();
                }

                if (wire_info["subgroups"].IsDefined()) {
                    auto subgroups = wire_info["subgroups"].as<std::vector<std::vector<size_t>>>();

                    if (ortho && curve) {
                        wocRen.props.push_back(WireframeProp::build(
                            group, root, curve, ortho, color, subgroups, exclude
                        ));
                    } else if (ortho) {
                        woRen.props.push_back(WireframeProp::build(
                            group, root, curve, ortho, color, subgroups, exclude
                        ));
                    } else if (curve) {
                        wscRen.props.push_back(WireframeProp::build(
                            group, root, curve, ortho, color, subgroups, exclude
                        ));
                    } else {
                        wsRen.props.push_back(WireframeProp::build(
                            group, root, curve, ortho, color, subgroups, exclude
                        ));
                    }
                } else {
                    auto combos = Combos<size_t>(gens, 1);

                    if (ortho && curve) {
                        wocRen.props.push_back(WireframeProp::build(
                            group, root, curve, ortho, color, combos, exclude
                        ));
                    } else if (ortho) {
                        woRen.props.push_back(WireframeProp::build(
                            group, root, curve, ortho, color, combos, exclude
                        ));
                    } else if (curve) {
                        wscRen.props.push_back(WireframeProp::build(
                            group, root, curve, ortho, color, combos, exclude
                        ));
                    } else {
                        wsRen.props.push_back(WireframeProp::build(
                            group, root, curve, ortho, color, combos, exclude
                        ));
                    }
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

        woRen.render();
        wsRen.render();
        wocRen.render();
        wscRen.render();

        sRen.render();

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
