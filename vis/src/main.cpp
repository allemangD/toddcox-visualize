#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <iostream>
#include <fstream>

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <gl/debug.hpp>
#include <gl/buffer.hpp>
#include <gl/shader.hpp>
#include <gl/vertexarray.hpp>

#include <tc/groups.hpp>
#include <tc/core.hpp>

#include <geo/mirror.hpp>
#include <geo/solver.hpp>

#include "render/pointrender.hpp"

Eigen::Matrix4f rotor(int u, int v, float rad) {
    Eigen::Matrix4f res = Eigen::Matrix4f::Identity();
    res(u, u) = res(v, v) = cosf(rad);
    res(u, v) = res(v, u) = sinf(rad);
    res(u, v) *= -1;
    return res;
}

template<typename T_>
T_ mix(const T_ &a, const T_ &b, const typename T_::Scalar &x) {
    return a * (1 - x) + b * x;
}

void show_overlay(State &state) {
    static std::string gl_vendor = (const char *) glGetString(GL_VENDOR);
    static std::string gl_renderer = (const char *) glGetString(GL_RENDERER);
    static std::string gl_version = (const char *) glGetString(GL_VERSION);
    static std::string glsl_version = (const char *) glGetString(GL_SHADING_LANGUAGE_VERSION);

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

    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.35f * style.Alpha);
    ImGui::Begin("Controls", nullptr, window_flags);

    ImGuiIO &io = ImGui::GetIO();
    ImGui::Text("FPS          | %.2f", io.Framerate);

    ImGui::Separator();

    ImGui::ColorEdit3("Background", state.bg.data(), ImGuiColorEditFlags_Float);
    ImGui::ColorEdit3("Foreground", state.fg.data(), ImGuiColorEditFlags_Float);
//    ImGui::ColorEdit3("Wireframe", state.wf.data(), ImGuiColorEditFlags_Float);

//    ImGui::Checkbox("Show RGBY axis colors", &state.color_axes);

    if (io.MouseDown[0] && !io.WantCaptureMouse) {
        Eigen::Matrix4f rot = Eigen::Matrix4f::Identity();
        Eigen::Vector2f del{io.MouseDelta.x, io.MouseDelta.y};
        del /= 200.0f;

        if (io.KeyShift) {
            del /= 5.0f;
        }

        if (io.KeyCtrl) {
            Eigen::Matrix4f rx = rotor(0, 3, -del.x());
            Eigen::Matrix4f ry = rotor(1, 3, del.y());
            rot = rx * ry;
        } else {
            Eigen::Matrix4f rx = rotor(0, 2, -del.x());
            Eigen::Matrix4f ry = rotor(1, 2, del.y());
            rot = rx * ry;
        }

        state.view = rot * state.view;
    }

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

int run(GLFWwindow *window, ImGuiContext *context) {
    State state;

    PointCloud pc;
    LineCloud lc;
    PointRenderer<Eigen::Vector4f> point_render;
    LineRenderer<Eigen::Vector4f> line_render;

    {
        tc::Group group = tc::coxeter("3 4 3");
        Eigen::Vector4f coords{1, 1, 1, 1};

        auto cosets = tc::solve(group, {}, 1000000);
        
        auto mirrors = mirror<4>(group);

        auto corners = plane_intersections(mirrors);
        auto start = barycentric(corners, coords);
        start.normalize();
        
        auto points = cosets.path.walk<vec4, vec4>(start, mirrors, reflect<vec4>);

        std::vector<Prims<2>> edges = hull<2>(group, std::vector<std::vector<tc::Gen>>{{0}, {1}, {2}, {3}}, {});
        
        pc.upload(points);
        lc.upload(points, edges);
    }

    glEnable(GL_DEPTH_TEST);
    glPointSize(2);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        show_overlay(state);
        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(state.bg[0], state.bg[1], state.bg[2], state.bg[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto aspect = (float) display_h / (float) display_w;
        state.proj = Eigen::AlignedScaling3f(aspect, 1.0, -0.6);

        point_render.draw(pc, state);
        line_render.draw(lc, state);
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    return EXIT_SUCCESS;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW:Failed initialization" << std::endl;
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    auto *window = glfwCreateWindow(1280, 720, "Cosets Visualization", nullptr, nullptr);
    if (!window) {
        std::cerr << "GLFW:Failed to create window" << std::endl;
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

#ifndef NDEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(log_gl_debug_callback, nullptr);
    glDebugMessageControl(
        GL_DONT_CARE, GL_DEBUG_TYPE_OTHER,
        GL_DEBUG_SEVERITY_NOTIFICATION,
        0, nullptr, GL_FALSE
    );
#endif

    IMGUI_CHECKVERSION();
    auto *context = ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    set_style();

    int exit_code = EXIT_SUCCESS;

    try {
        exit_code = run(window, context);
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
