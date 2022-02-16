#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <iostream>
#include <fstream>

#include "gldebug.hpp"

#include <ml/meshlib.hpp>
#include <ml/meshlib_json.hpp>

struct State {
    float bg[4] = {0.45f, 0.55f, 0.60f, 1.00f};
    float fg[4] = {0.19f, 0.86f, 0.33f, 1.00f};
};

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

    ImGui::ColorEdit3("Background", state.bg, ImGuiColorEditFlags_Float);
    ImGui::ColorEdit3("Foreground", state.fg, ImGuiColorEditFlags_Float);

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

    auto mesh = ml::CubeMesh(0.25f);
//    auto mesh = ml::read("circle.pak");

    auto dynamic = (ml::DynamicMesh) mesh;

    GLuint vao;
    glCreateVertexArrays(1, &vao);

    GLuint vbo;
    glCreateBuffers(1, &vbo);

    constexpr size_t point_scalar_size = sizeof(ml::DynamicMesh::PointsType::Scalar);
    constexpr size_t cell_scalar_size = sizeof(ml::DynamicMesh::CellsType::Scalar);

    glNamedBufferData(
        vbo,
        (GLsizeiptr) (point_scalar_size * dynamic.points().size()),
        dynamic.points().data(),
        GL_STATIC_DRAW
    );
    glEnableVertexArrayAttrib(vao, 0);
    glVertexArrayVertexBuffer(vao, 0,
        vbo, 0,
        (GLsizeiptr) (point_scalar_size * dynamic.points().rows())
    );
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);

    GLuint ibo;
    glCreateBuffers(1, &ibo);
    glGetError();
    glNamedBufferData(
        ibo,
        (GLsizeiptr) (cell_scalar_size * dynamic.cells().size()),
        dynamic.cells().data(),
        GL_STATIC_DRAW
    );
    glVertexArrayElementBuffer(vao, ibo);

    auto wire_mesh = ml::WireCubeMesh(4, 0.33f);
    auto wire_dynamic = (ml::DynamicMesh) wire_mesh;

    GLuint wire_vao;
    glCreateVertexArrays(1, &wire_vao);

    GLuint wire_vbo;
    glCreateBuffers(1, &wire_vbo);

    glNamedBufferData(
        wire_vbo,
        (GLsizeiptr) (point_scalar_size * wire_dynamic.points().size()),
        wire_dynamic.points().data(),
        GL_STATIC_DRAW
    );
    glEnableVertexArrayAttrib(wire_vao, 0);
    glVertexArrayVertexBuffer(wire_vao, 0,
                              wire_vbo, 0,
                              (GLsizeiptr) (point_scalar_size * wire_dynamic.points().rows())
    );
    glVertexArrayAttribFormat(wire_vao, 0, 4, GL_FLOAT, GL_FALSE, 0);

    GLuint wire_ibo;
    glCreateBuffers(1, &wire_ibo);
    glGetError();
    glNamedBufferData(
        wire_ibo,
        (GLsizeiptr) (cell_scalar_size * wire_dynamic.cells().size()),
        wire_dynamic.cells().data(),
        GL_STATIC_DRAW
    );
    glVertexArrayElementBuffer(wire_vao, wire_ibo);

    std::ifstream vs_file("res/shaders/main.vert.glsl");
    std::string vs_src(
        (std::istreambuf_iterator<char>(vs_file)),
        std::istreambuf_iterator<char>()
    );
    vs_file.close();
    const char *vs_str = vs_src.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_str, nullptr);
    glCompileShader(vs);

    std::ifstream wire_vs_file("res/shaders/4d.vert.glsl");
    std::string wire_vs_src(
        (std::istreambuf_iterator<char>(wire_vs_file)),
        std::istreambuf_iterator<char>()
    );
    wire_vs_file.close();
    const char *wire_vs_str = wire_vs_src.c_str();

    GLuint wire_vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(wire_vs, 1, &wire_vs_str, nullptr);
    glCompileShader(wire_vs);

    std::ifstream fs_file("res/shaders/main.frag.glsl");
    std::string fs_src(
        (std::istreambuf_iterator<char>(fs_file)),
        std::istreambuf_iterator<char>()
    );
    fs_file.close();
    const char *fs_str = fs_src.c_str();

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_str, nullptr);
    glCompileShader(fs);

    GLuint pgm = glCreateProgram();
    glAttachShader(pgm, vs);
    glAttachShader(pgm, fs);
    glLinkProgram(pgm);

    GLuint wire_pgm = glCreateProgram();
    glAttachShader(wire_pgm, wire_vs);
    glAttachShader(wire_pgm, fs);
    glLinkProgram(wire_pgm);

    GLint link_status;
    glGetProgramiv(pgm, GL_LINK_STATUS, &link_status);
    if (!link_status) {
        std::cerr << "Program link failed." << std::endl;
        GLint vs_comp_status, fs_comp_status;
        glGetShaderiv(vs, GL_COMPILE_STATUS, &vs_comp_status);
        glGetShaderiv(fs, GL_COMPILE_STATUS, &fs_comp_status);
        std::cerr << "vs compiled: " << std::boolalpha << (bool) vs_comp_status << std::endl;
        std::cerr << "fs compiled: " << std::boolalpha << (bool) fs_comp_status << std::endl;
        return EXIT_FAILURE;
    }

    glGetProgramiv(wire_pgm, GL_LINK_STATUS, &link_status);
    if (!link_status) {
        std::cerr << "Wire program link failed." << std::endl;
        GLint vs_comp_status, fs_comp_status;
        glGetShaderiv(vs, GL_COMPILE_STATUS, &vs_comp_status);
        glGetShaderiv(fs, GL_COMPILE_STATUS, &fs_comp_status);
        std::cerr << "vs compiled: " << std::boolalpha << (bool) vs_comp_status << std::endl;
        std::cerr << "fs compiled: " << std::boolalpha << (bool) fs_comp_status << std::endl;
        return EXIT_FAILURE;
    }

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
        glUniform4fv(0, 1, state.fg);
        glUniform1f(1, (GLfloat) glfwGetTime());
        glUniformMatrix4fv(2, 1, false, proj.data());
        glDrawElements(GL_TRIANGLES, (GLsizei) dynamic.cells().size(), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
        glUseProgram(0);

        glUseProgram(wire_pgm);
        glBindVertexArray(wire_vao);
        glUniform4fv(0, 1, state.fg);
        glUniform1f(1, (GLfloat) glfwGetTime());
        glUniformMatrix4fv(2, 1, false, proj.data());
        glDrawElements(GL_LINES, (GLsizei) wire_dynamic.cells().size(), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
        glUseProgram(0);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
    glDeleteVertexArrays(1, &vao);

    glDeleteBuffers(1, &wire_vbo);
    glDeleteBuffers(1, &wire_ibo);
    glDeleteVertexArrays(1, &wire_vao);

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
