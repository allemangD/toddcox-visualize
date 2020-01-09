#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <tc/groups.hpp>

#include "util.hpp"
#include "mirror.hpp"
#include "geometry.hpp"

#ifdef _WIN32
extern "C" {
__attribute__((unused)) __declspec(dllexport) int NvOptimusEnablement = 0x00000001;
}
#endif

int main(int argc, char *argv[]) {
    //region init window

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return EXIT_FAILURE;
    }

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
    glfwSwapInterval(0);

    //endregion

    std::cout << utilInfo();

    const std::string VS_SOURCE =
        "#version 430\n"
        "layout(location=0) uniform mat4 proj;"
        "layout(location=1) uniform mat4 view;"
        ""
        "layout(location=0) in vec4 pos;"
        ""
        "out vec4 vpos;"
        ""
        "void main() {"
        "   int i = gl_VertexID;"
        "   vpos = view * pos;"
        "   gl_Position = proj * vec4(vpos.xyz / (1), 1);"
        //                "   gl_Position = proj * vec4(vpos.xyz / (1 - vpos.w), 1);"
        "   gl_PointSize = 5;"
        "}";

    const std::string FS_SOURCE =
        "#version 430\n"
        "layout(location=2) uniform vec3 c;"
        ""
        "in vec4 vpos;"
        ""
        "out vec4 color;"
        ""
        "void main() {"
        "   float d = smoothstep(-2, 2, vpos.z);"
        "   vec3 off = 1.04 * vec3(0, 2, 4) + 2 * vec3(vpos.w);"
        "   color = vec4(c * d, 1);"
        "}";

    //region init shaders
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    utilShaderSource(vs, {VS_SOURCE});
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    utilShaderSource(fs, {FS_SOURCE});

    GLuint pgm = glCreateProgram();
    glAttachShader(pgm, vs);
    glAttachShader(pgm, fs);
    glLinkProgram(pgm);

    GLint status;

    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
    if (!status) {
        std::cerr << utilShaderInfoLog(vs) << "\n=========\n" << std::endl;
    }

    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
    if (!status) {
        std::cerr << utilShaderInfoLog(fs) << "\n=========\n" << std::endl;
    }

    glGetProgramiv(pgm, GL_LINK_STATUS, &status);
    if (!status) {
        std::cerr << utilProgramInfoLog(pgm) << "\n=========\n" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }
    //endregion

    auto group = tc::group::H(3);
    auto res = group.solve();
    auto mirrors = mirror(group);
    std::cout << "Solved " << res.size() << std::endl;
    std::cout << "Mirror lengths:" << std::endl;
    for (const auto &m : mirrors) {
        std::cout << glm::length(m) << " (" << m.x << " " << m.y << " " << m.z << " " << m.w << ")" << std::endl;
    }

    auto corners = plane_intersections(mirrors);
    auto start = barycentric(corners, {1.00, 1.00, 1.00, 1.00});
    auto points = res.path.walk<glm::vec4, glm::vec4>(start, mirrors, reflect);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * points.size(), &points[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, false, 0, nullptr);

    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(pgm);
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_POINT_SMOOTH);
        glEnable(GL_DEPTH_TEST);

        //region uniforms
        auto aspect = (float) width / (float) height;
        auto pheight = 1.4f;
        auto pwidth = aspect * pheight;
        glm::mat4 proj = glm::ortho(-pwidth, pwidth, -pheight, pheight, -100.0f, 100.0f);
        glUniformMatrix4fv(0, 1, false, glm::value_ptr(proj));

        auto t = (float) glfwGetTime() / 3;
        auto view = glm::identity<glm::mat4>();
        view = glm::rotate(view, t / 1, glm::vec3(0, 1, 0));
        view = glm::rotate(view, t / 3, glm::vec3(0, 0, 1));
        view = glm::rotate(view, t / 4, glm::vec3(1, 0, 0));
        glUniformMatrix4fv(1, 1, false, glm::value_ptr(view));
        //endregion

        glUniform3f(2, 1.0f, 1.0f, 1.0f);
        glDrawArrays(GL_POINTS, 0, points.size());

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}
