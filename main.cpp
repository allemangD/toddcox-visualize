#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <iostream>

#include "gldebug.hpp"

void show_overlay() {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_None
        | ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoFocusOnAppearing
        | ImGuiWindowFlags_NoNav
        | ImGuiWindowFlags_NoMove;
    ImGuiStyle &style = ImGui::GetStyle();
    const auto PAD = style.DisplaySafeAreaPadding;
    const auto *viewport = ImGui::GetMainViewport();
    const auto work_pos = viewport->WorkPos;
    const auto work_size = viewport->WorkSize;
    const auto window_pos = ImVec2(
        work_pos.x + PAD.x,
        work_pos.y + PAD.y
    );
    const auto window_pos_pivot = ImVec2(
        0.0f,
        0.0f
    );
    ImGui::SetNextWindowPos(
        window_pos,
        ImGuiCond_Always,
        window_pos_pivot
    );
    ImGui::SetNextWindowBgAlpha(0.35f * style.Alpha);
    if (ImGui::Begin("Graphics Information", nullptr, window_flags)) {
        ImGui::BeginTable("graphics", 2, ImGuiTableFlags_BordersInnerV);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("GL Vendor");
        ImGui::TableNextColumn();
        ImGui::Text("%s", glGetString(GL_VENDOR));
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("GL Renderer");
        ImGui::TableNextColumn();
        ImGui::Text("%s", glGetString(GL_RENDERER));
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("GL Version");
        ImGui::TableNextColumn();
        ImGui::Text("%s", glGetString(GL_VERSION));
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("GLSL Version");
        ImGui::TableNextColumn();
        ImGui::Text("%s", glGetString(GL_SHADING_LANGUAGE_VERSION));
        ImGui::EndTable();
        ImGui::End();
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW :: failed initialization" << std::endl;
        exit(EXIT_FAILURE);
    }

    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    auto *window = glfwCreateWindow(1280, 720, "Hello OpenGL", nullptr, nullptr);
    if (!window) {
        std::cerr << "GLFW :: failed to create window" << std::endl;
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    io.Fonts->AddFontDefault();

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        show_overlay();
        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w,
            clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    exit(EXIT_SUCCESS);
}
