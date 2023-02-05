#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>

#include <tc/groups.hpp>

#include "util.hpp"
#include "mirror.hpp"
#include "solver.hpp"

#include <cgl/debug.hpp>
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

Matrices build(GLFWwindow* window, State &state) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    auto aspect = (float) width / (float) height;
    auto pheight = 1.4f;
    auto pwidth = aspect * pheight;
    Eigen::Matrix4f proj = orthographic(-pwidth, pwidth, -pheight, pheight, -10.0f, 10.0f);

    if (!glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
        state.st += state.time_delta / 8;
    }

    Eigen::Matrix4f view;
    view.setIdentity();

    if (state.dimension < 4) {
        view *= rot<4>(2, 3, M_PI_2f32 + 0.01f);
    }

    if (state.dimension > 1) {
        view *= rot<4>(0, 1, state.st * .40f);
    }
    if (state.dimension > 2) {
        view *= rot<4>(0, 2, state.st * .20f);
        view *= rot<4>(1, 2, state.st * .50f);
    }
    if (state.dimension > 3) {
        view *= rot<4>(0, 3, state.st * 1.30f);
        view *= rot<4>(1, 3, state.st * .25f);
        view *= rot<4>(2, 3, state.st * 1.42f);
    }

    return Matrices(proj, view);
}

template<class C>
std::vector<vec4> points(const tc::Group<> &group, const C &coords) {
    auto cosets = group.solve();
    auto mirrors = mirror<5>(group);
    auto corners = plane_intersections(mirrors);

    vec5 coord = coords;
    auto start = corners * coord;

    tc::Path<vec5> path(cosets, mirrors.colwise());

    Eigen::Array<float, 5, Eigen::Dynamic> higher(5, path.order());
    path.walk(start, Reflect(), higher.matrix().colwise().begin());
//    std::vector<vec5> higher(path.order());
//    path.walk(start, Reflect(), higher.begin());

//    Eigen::Array4Xf lower = higher.topRows<4>().rowwise() / (1 - higher.bottomRows<1>());
    Eigen::Array4Xf lower = Stereo()(higher);

    std::vector<vec4> vec(lower.cols());
    std::copy(lower.colwise().begin(), lower.colwise().end(), vec.begin());

    return vec;
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
            for (const auto &prop: props) {
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

        auto pts = points(g, coords);
        res.vbo.put(pts.begin(), pts.end());
        auto inds = merge<N>(hull<N>(g, all_sg_gens, exclude));
        res.ibo.put(inds.begin(), inds.end());
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
        glProgramUniform3fv(solid, 2, 1, prop.color.data());
//        glProgramUniform3f(solid, 2, 1.f, 1.f, 1.f);
        prop.vao.bound([&]() {
            glDrawArrays(GL_POINTS, 0, prop.ibo.count() * N);
        });
    }
};


void run(const std::string &config_file, GLFWwindow* window) {
#ifndef NDEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(log_gl_debug_callback, nullptr);
#endif

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    SliceRenderer<4> sRen{};

    State state{};
    glfwSetWindowUserPointer(window, &state);

    std::cout << "building..." << std::endl;
    {
        auto group = tc::schlafli({5, 3, 3, 2});
        auto gens = generators(group);
        vec5 root;
        root << 0.80, 0.09, 0.09, 0.09, 0.04;
        vec3 color;
        color << 0.90, 0.90, 0.90;

        std::vector<std::vector<size_t>> exclude{
            {0, 1, 2},
        };

        auto combos = combinations(gens, 3);

        sRen.props.push_back(SliceProp<4>::build(
            group, root, color, combos, exclude
        ));
    }
    std::cout << "built" << std::endl;

    state.dimension = 4;

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

        sRen.render();

        glfwSwapInterval(2);
        glfwSwapBuffers(window);

        glfwPollEvents();
    }
}

int main(int argc, char* argv[]) {
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
