#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <iostream>
#include <fstream>

#include "gldebug.hpp"

#include "meshlib.hpp"
#include "meshlib_json.hpp"

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

    auto mesh = ml::CubeMesh(0.5f);
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

    const char *vs_src = "#version 440\n"
                         "layout(location=1) uniform float time;"
                         "layout(location=2) uniform mat4 proj;"
                         "layout(location=0) in vec3 pos;"
                         "void main() {"
                         "  float c2 = cos(time * 0.2);"
                         "  float s2 = sin(time * 0.2);"
                         "  float c3 = cos(time * 0.3);"
                         "  float s3 = sin(time * 0.3);"
                         "  mat4 r1 = mat4("
                         "     c2,  -s2, 0.0, 0.0,"
                         "     s2,   c2, 0.0, 0.0,"
                         "    0.0,  0.0, 1.0, 0.0,"
                         "    0.0,  0.0, 0.0, 1.0"
                         "  );"
                         "  mat4 r2 = mat4("
                         "     c3,  0.0, -s3, 0.0,"
                         "    0.0,  1.0, 0.0, 0.0,"
                         "     s3,  0.0,  c3, 0.0,"
                         "    0.0,  0.0, 0.0, 1.0"
                         ");"
                         "  gl_Position = proj * r2 * r1 * vec4(pos, 1.0);"
                         "}";
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_src, nullptr);
    glCompileShader(vs);

    const char *fs_src = "#version 440\n"
                         "layout(location=0) uniform vec4 ucol;"
                         "layout(location=0) out vec4 col;"
                         "void main() {"
                         "  float d = 1.0 - gl_FragCoord.z;"
                         "  d = (d - 0.5) / 0.7 + 0.5;"
                         "  col = ucol;"
                         "  col.xyz *= d;"
                         "}";
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_src, nullptr);
    glCompileShader(fs);

    GLuint pgm = glCreateProgram();
    glAttachShader(pgm, vs);
    glAttachShader(pgm, fs);
    glLinkProgram(pgm);

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

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
    glDeleteVertexArrays(1, &vao);

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
