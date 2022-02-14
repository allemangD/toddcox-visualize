#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <iostream>
#include <fstream>

#include "gl/debug.hpp"
#include "gl/buffer.hpp"
#include "gl/shader.hpp"
#include "gl/vertexarray.hpp"

#include <ml/meshlib.hpp>
#include <ml/meshlib_json.hpp>

struct State {
    Eigen::Vector4f bg{0.07f, 0.09f, 0.10f, 1.00f};
    Eigen::Vector4f fg{0.71f, 0.53f, 0.94f, 1.00f};
    Eigen::Vector4f wf{0.95f, 0.95f, 0.95f, 1.00f};

    Eigen::Vector4f R{1.00f, 0.00f, 0.00f, 1.00f};
    Eigen::Vector4f G{0.00f, 1.00f, 0.00f, 1.00f};
    Eigen::Vector4f B{0.00f, 0.00f, 1.00f, 1.00f};
    Eigen::Vector4f Y{1.20f, 1.20f, 0.00f, 1.00f};

    Eigen::Matrix4f rot = Eigen::Matrix4f::Identity();

    bool color_axes = false;
};

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
    ImGui::ColorEdit3("Wireframe", state.wf.data(), ImGuiColorEditFlags_Float);

    ImGui::Checkbox("Show RGBY axis colors", &state.color_axes);

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

        state.rot = rot * state.rot;
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

    Buffer<GLuint> ind_buf;
    Buffer<Eigen::Vector3f> vert_buf;
    Buffer<Eigen::Vector3f> vert2_buf;

    VertexArray<Eigen::Vector3f> vao(vert_buf);
    glVertexArrayElementBuffer(vao, ind_buf);

    auto mesh = ml::CubeMesh(0.25f);

    auto ind_data = mesh.cells();
    auto ind_flat = ind_data.reshaped();
    auto elements = ind_buf.upload(ind_flat.begin(), ind_flat.end(), GL_STATIC_DRAW);

    // todo add <Dim, Rank> to DynamicMesh
    // need to do weird piping because dynamicmesh returns dynamic-sized matrix, VertexArray requires a static-sized matrix
    auto vert_data_dyn = mesh.points();
    Eigen::Ref<Eigen::Matrix3Xf> vert_data(vert_data_dyn);
    auto vert_flat = vert_data.colwise();
    vert_buf.upload(vert_flat.begin(), vert_flat.end(), GL_STATIC_DRAW);

    auto mesh2 = ml::CubeMesh(0.5f);
    auto vert2_data_dyn = mesh2.points();
    Eigen::Ref<Eigen::Matrix3Xf> vert2_data(vert2_data_dyn);
    auto vert2_flat = vert2_data.colwise();
    vert2_buf.upload(vert2_flat.begin(), vert2_flat.end(), GL_STATIC_DRAW);

    auto mesh4d = ml::WireCubeMesh(4, 0.33f);

    Buffer<GLuint> ind4d_buf;
    Buffer<Eigen::Vector4f> vert4d_buf;
    VertexArray<Eigen::Vector4f> vao4d(vert4d_buf);
    glVertexArrayElementBuffer(vao4d, ind4d_buf);

    auto ind4d_data = mesh4d.cells();
    auto ind4d_flat = ind4d_data.reshaped();
    auto elements4d = ind4d_buf.upload(ind4d_flat.begin(), ind4d_flat.end(), GL_STATIC_DRAW);

    auto vert4d_data_dyn = mesh4d.points();
    Eigen::Ref<Eigen::Matrix4Xf> vert4d_data(vert4d_data_dyn);
    auto vert4d_flat = vert4d_data.colwise();
    vert4d_buf.upload(vert4d_flat.begin(), vert4d_flat.end(), GL_STATIC_DRAW);

    VertexShader vs(std::ifstream("res/shaders/main.vert.glsl"));
    VertexShader vs4d(std::ifstream("res/shaders/4d.vert.glsl"));
    FragmentShader fs(std::ifstream("res/shaders/main.frag.glsl"));

    Program pgm(vs, fs);
    Program pgm4d(vs4d, fs);

    glEnable(GL_DEPTH_TEST);

    Eigen::Projective3f proj;

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
        proj = Eigen::AlignedScaling3f(aspect, 1.0, -1.0);

        glUseProgram(pgm);
        glBindVertexArray(vao);
        glUniform4fv(0, 1, state.fg.data());
        glUniform1f(1, (GLfloat) glfwGetTime());
        glUniformMatrix4fv(2, 1, false, proj.data());
        glUniformMatrix4fv(3, 1, false, state.rot.data());
        glDrawElements(GL_TRIANGLES, elements, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
        glUseProgram(0);

        glUseProgram(pgm4d);
        glBindVertexArray(vao4d);
        glUniform4fv(0, 1, state.wf.data());
        glUniform1f(1, (GLfloat) glfwGetTime());
        glUniformMatrix4fv(2, 1, false, proj.data());
        glUniformMatrix4fv(3, 1, false, state.rot.data());

        if (state.color_axes) {
            auto factor = 0.7f;
            auto x = mix(state.wf, state.R, factor);
            auto y = mix(state.wf, state.G, factor);
            auto z = mix(state.wf, state.B, factor);
            auto w = mix(state.wf, state.Y, factor);

            glUniform4fv(0, 1, x.data());
            glDrawElementsBaseVertex(GL_LINES, 2, GL_UNSIGNED_INT, (void *) (sizeof(GLuint) * 0), 0);
            glUniform4fv(0, 1, y.data());
            glDrawElementsBaseVertex(GL_LINES, 2, GL_UNSIGNED_INT, (void *) (sizeof(GLuint) * 2), 0);
            glUniform4fv(0, 1, z.data());
            glDrawElementsBaseVertex(GL_LINES, 2, GL_UNSIGNED_INT, (void *) (sizeof(GLuint) * 8), 0);
            glUniform4fv(0, 1, w.data());
            glDrawElementsBaseVertex(GL_LINES, 2, GL_UNSIGNED_INT, (void *) (sizeof(GLuint) * 24), 0);
        }

        glUniform4fv(0, 1, state.wf.data());
        glDrawElements(GL_LINES, elements4d, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
        glUseProgram(0);

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
