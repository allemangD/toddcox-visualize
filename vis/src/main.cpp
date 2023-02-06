#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>

#include <tc/groups.hpp>

#include "util.hpp"
#include "mirror.hpp"

#include "comps.hpp"
#include "fmt/core.h"

#include <shaders.hpp>

#ifndef NDEBUG
#include <cgl/debug.hpp>
#endif

#ifdef _WIN32
extern "C" {
__attribute__((unused)) __declspec(dllexport) int NvOptimusEnablement = 0x00000001;
}
#endif

#ifndef M_PI_2f32
#define M_PI_2f32 3.14159265358979323846f
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

    entt::registry registry;
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

    static bool init = false;
    static Eigen::Vector2d last;
    Eigen::Vector2d pos;

    glfwGetCursorPos(window, &pos.x(), &pos.y());
    if (!init) {
        last = pos;
        init = true;
    }
    Eigen::Vector2d dpos = pos - last;
    last = pos;

    static Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    if (state.dimension < 4) {
        view *= rot<4>(2, 3, M_PI_2f32 + 0.01f);
    }

    if (glfwGetMouseButton(window, 0)) {
        float scale = 0.005;
        auto rotate = rotor(
            Eigen::Vector4f{0, 0, 1, 0},
            Eigen::Vector4f{
                dpos.x() * scale,
                -dpos.y() * scale,
                1, 0
            }.normalized()
        );
        view = rotate * view;
    }

    return Matrices(proj, view);
}

void run(const std::string &config_file, GLFWwindow* window) {
#ifndef NDEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(log_gl_debug_callback, nullptr);
#endif

    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vis::SliceRenderer renderer;

    State state{};
    glfwSetWindowUserPointer(window, &state);

    auto &registry = state.registry;
    state.dimension = 4;

    auto entity = registry.create();

    registry.emplace<vis::Group>(
        entity,
        tc::schlafli({5, 3, 3, 2}),
        vec5{0.80, 0.09, 0.09, 0.09, 0.09},
        vec3{0.90, 0.90, 0.90},
        std::vector<std::vector<size_t>>{{0, 1, 2},
                                         {0, 3, 4},
                                         {1, 3, 4},
                                         {2, 3, 4}}
    );
    registry.emplace<vis::VBOs>(entity);

    vis::upload_groups(registry);

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

        ubo.put(build(window, state));

        {
            auto &tform = registry.get<vis::VBOs>(entity).tform;

            tform.linear().setIdentity();

            if (state.dimension > 1) {
                tform.linear() *= rot<4>(0, 1, state.st * .40f);
            }
            if (state.dimension > 2) {
                tform.linear() *= rot<4>(0, 2, state.st * .20f);
                tform.linear() *= rot<4>(1, 2, state.st * .50f);
            }
            if (state.dimension > 3) {
                tform.linear() *= rot<4>(0, 3, state.st * 1.30f);
                tform.linear() *= rot<4>(1, 3, state.st * .25f);
                tform.linear() *= rot<4>(2, 3, state.st * 1.42f);
            }

            tform.translation().w() = std::sin(time * 0.3f) * 1.0f;
        }

        vis::upload_ubo(registry);

        renderer(registry);

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
    glfwWindowHint(GLFW_SAMPLES, 8);
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
