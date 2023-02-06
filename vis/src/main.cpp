#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

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
    int dimension;

    float time = 0;

    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    entt::registry registry;
};

Matrices build(GLFWwindow* window, State &state, ImGuiContext* ctx) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    auto aspect = (float) width / (float) height;
    auto pheight = 1.4f;
    auto pwidth = aspect * pheight;
    Eigen::Matrix4f proj = orthographic(-pwidth, pwidth, -pheight, pheight, -10.0f, 10.0f);

    auto &io = ImGui::GetIO();

    if (io.MouseDown[0] && !io.WantCaptureMouse) {
        Eigen::Vector4f src{0, 0, 1, 0};
        Eigen::Vector4f dst{io.MouseDelta.x, -io.MouseDelta.y, 300, 0};
        dst.normalize();

        auto rotate = rotor(src, dst);
        state.view = rotate * state.view;
    }

    return Matrices(proj, state.view);
}

void show_overlay(State &state) {
    static std::string gl_vendor = (const char*) glGetString(GL_VENDOR);
    static std::string gl_renderer = (const char*) glGetString(GL_RENDERER);
    static std::string gl_version = (const char*) glGetString(GL_VERSION);
    static std::string glsl_version = (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION);

    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoMove;

    ImGuiStyle &style = ImGui::GetStyle();
    const auto PAD = style.DisplaySafeAreaPadding;
    auto window_pos = PAD;

    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.35f * style.Alpha);
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_Appearing);
    ImGui::Begin("Graphics Information", nullptr, window_flags);
    ImGui::Text("GL Vendor    | %s", gl_vendor.c_str());
    ImGui::Text("GL Renderer  | %s", gl_renderer.c_str());
    ImGui::Text("GL Version   | %s", gl_version.c_str());
    ImGui::Text("GLSL Version | %s", glsl_version.c_str());

    auto v2 = ImGui::GetWindowSize();
    window_pos.y += v2.y + PAD.y;
    ImGui::End();
}

void set_style() {
    ImGui::StyleColorsDark();

    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding = 4;
    style.FrameRounding = 2;
    style.DisplaySafeAreaPadding.x = 10;
    style.DisplaySafeAreaPadding.y = 10;
}

void run(GLFWwindow* window, ImGuiContext* ctx) {
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
        std::vector<std::vector<size_t>>{
            {0, 1, 2},

            {0, 3, 4},
            {1, 3, 4},
            {2, 3, 4},
        }
    );
    registry.emplace<vis::VBOs>(entity);

    vis::upload_groups(registry);

    auto ubo = cgl::Buffer<Matrices>();
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        auto io = ImGui::GetIO();
        if (io.KeysDown[GLFW_KEY_ESCAPE]) {
            glfwSetWindowShouldClose(window, true);
            continue;
        }

        ImGui::NewFrame();
        show_overlay(state);
        ImGui::Render();

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ubo.put(build(window, state, ctx));

        {
            auto &tform = registry.get<vis::VBOs>(entity).tform;

            if (!io.KeysDown[GLFW_KEY_SPACE]) {
                float speed = 1.0 / 8.0;
                if (io.KeysDown[GLFW_KEY_LEFT_SHIFT] | io.KeysDown[GLFW_KEY_RIGHT_SHIFT]) {
                    speed /= 4;
                }
                state.time += io.DeltaTime * speed;
            }

            tform.linear().setIdentity();

            if (state.dimension > 1) {
                tform.linear() *= rot<4>(0, 1, state.time * .40f);
            }
            if (state.dimension > 2) {
                tform.linear() *= rot<4>(0, 2, state.time * .20f);
                tform.linear() *= rot<4>(1, 2, state.time * .50f);
            }
            if (state.dimension > 3) {
                tform.linear() *= rot<4>(0, 3, state.time * 1.30f);
                tform.linear() *= rot<4>(1, 3, state.time * .25f);
                tform.linear() *= rot<4>(2, 3, state.time * 1.42f);
            }

            tform.translation().w() = std::sin(state.time * 1.4) * 1.0;
        }

        vis::upload_ubo(registry);

        renderer(registry);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapInterval(2);
        glfwSwapBuffers(window);
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

    glfwWindowHint(GLFW_SAMPLES, 8);

//    glfwWindowHint(GLFW_DECORATED, false);
//    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, true);

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

#ifndef NDEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(log_gl_debug_callback, nullptr);
#endif

    IMGUI_CHECKVERSION();
    auto* context = ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    set_style();

    int exit_code = EXIT_SUCCESS;

    try {
        run(window, context);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        exit_code = EXIT_FAILURE;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return exit_code;
}
