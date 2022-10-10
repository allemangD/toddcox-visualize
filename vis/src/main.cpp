#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <fmt/core.h>
#include <fstream>

#include "debug.hpp"

int run(GLFWwindow *window, ImGuiContext *ctx) {
    glClearColor(0.1, 0.1, 0.9, 1.0);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Hello There!", nullptr, ImGuiWindowFlags_None);
        ImGui::Text("General Kenobi.");
        ImGui::End();
        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    return EXIT_SUCCESS;
}

int main() {
    if (!glfwInit()) {
        fmt::print(stderr, "GLFW : Failed Initialization\n");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(
        1280, 720,
        "Cosets Visualization",
        nullptr, nullptr
    );

    if (!window) {
        fmt::print("GLFW : Failed to create window.\n");
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
    ImGuiContext *ctx = ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    int code = EXIT_SUCCESS;

    try {
        code = run(window, ctx);
    } catch (const std::exception &ex) {
        fmt::print(stderr, "{}\n", ex.what());
        code = EXIT_FAILURE;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return code;
}
