#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>

#include <tc/groups.hpp>

#include "util.hpp"
#include "mirror.hpp"
#include "geometry.hpp"

#ifdef _WIN32
extern "C" {
__attribute__((unused)) __declspec(dllexport) int NvOptimusEnablement = 0x00000001;
}
#endif

struct Matrices {
    glm::mat4 proj;
    glm::mat4 view;

    Matrices(const glm::mat4 &proj, const glm::mat4 &view)
        : proj(proj), view(view) {
    }
};

template<unsigned N>
struct MeshRef {
    GLuint vao;
    GLuint ibo;
    unsigned primitive_count;
    unsigned index_count;

    explicit MeshRef(const Mesh<N> &mesh) {
        vao = utilCreateVertexArray();
        ibo = utilCreateBuffer();
        primitive_count = mesh.size();
        index_count = primitive_count * N;

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, ibo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Primitive<N>) * primitive_count, &mesh.prims[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribIPointer(0, N, GL_INT, 0, nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
};

float factor(unsigned index, unsigned size) {
    auto num = (float) index;
    auto den = size > 1 ? (float) size - 1 : 1;
    return num / den;
}

Matrices build(GLFWwindow *window, float st) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    auto aspect = (float) width / (float) height;
    auto pheight = 1.4f;
    auto pwidth = aspect * pheight;
    glm::mat4 proj = glm::ortho(-pwidth, pwidth, -pheight, pheight, -10.0f, 10.0f);

    auto view = glm::identity<glm::mat4>();
    view *= utilRotate(0, 1, st * .40f);
    view *= utilRotate(0, 2, st * .20f);
    view *= utilRotate(0, 3, st * 1.30f);
    view *= utilRotate(1, 2, st * .50f);
    view *= utilRotate(1, 3, st * .25f);
    view *= utilRotate(2, 3, st * 1.42f);

    return Matrices(proj, view);
}

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
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);
    //endregion

    std::cout << utilInfo();

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    //region shaders
    GLuint slice_pipe;
    glCreateProgramPipelines(1, &slice_pipe);

    GLuint proj_pipe;
    glCreateProgramPipelines(1, &proj_pipe);

//    GLuint defer, direct_ortho, direct_stereo, slice, curve_stereo, solid;
    GLuint defer, direct_ortho, direct_stereo;
    GLuint slice, curve_ortho, curve_stereo;
    GLuint solid;

    try {
        defer = utilCreateShaderProgramFile(GL_VERTEX_SHADER, {"shaders/slice/deferred.vs.glsl"});
        direct_ortho = utilCreateShaderProgramFile(GL_VERTEX_SHADER, {"shaders/direct-ortho.vs.glsl"});
        direct_stereo = utilCreateShaderProgramFile(GL_VERTEX_SHADER, {"shaders/direct-stereo.vs.glsl"});

        slice = utilCreateShaderProgramFile(GL_GEOMETRY_SHADER, {"shaders/slice/slice.gm.glsl"});
        curve_stereo = utilCreateShaderProgramFile(GL_GEOMETRY_SHADER, {"shaders/curve-stereo.gm.glsl"});
        curve_ortho = utilCreateShaderProgramFile(GL_GEOMETRY_SHADER, {"shaders/curve-ortho.gm.glsl"});

        solid = utilCreateShaderProgramFile(GL_FRAGMENT_SHADER, {"shaders/solid.fs.glsl"});

        glUseProgramStages(slice_pipe, GL_VERTEX_SHADER_BIT, defer);
        glUseProgramStages(slice_pipe, GL_GEOMETRY_SHADER_BIT, slice);
        glUseProgramStages(slice_pipe, GL_FRAGMENT_SHADER_BIT, solid);

        glUseProgramStages(proj_pipe, GL_VERTEX_SHADER_BIT, direct_stereo);
//        glUseProgramStages(proj_pipe, GL_GEOMETRY_SHADER_BIT, curve_stereo);
        glUseProgramStages(proj_pipe, GL_FRAGMENT_SHADER_BIT, solid);

    } catch (const gl_error &e) {
        std::cerr << e.what() << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    //endregion

    //region points
    auto group = tc::group::H(4);
    auto res = group.solve();
    auto mirrors = mirror(group);

    auto corners = plane_intersections(mirrors);
//    auto start = barycentric(corners, {1.0f, 1.0f, 1.0f, 1.0f});
    auto start = barycentric(corners, {1.00f, 0.2f, 0.1f, 0.05f});
//    auto start = barycentric(corners, {0.05f, 0.1f, 0.2f, 1.00f});
    auto points = res.path.walk<glm::vec4, glm::vec4>(start, mirrors, reflect);

    auto g_gens = gens(group);

    const unsigned WIRES_N = 2;
    const GLenum WIRE_MODE = GL_LINES;
    std::vector<MeshRef<WIRES_N>> wires;

    for (const auto &sg_gens : Combos(g_gens, WIRES_N - 1)) {
        const auto s = triangulate<WIRES_N>(group, sg_gens).tile(group, g_gens, sg_gens);

        wires.emplace_back(s);
    }

    const unsigned SLICES_N = 4;
    const GLenum SLICES_MODE = GL_POINTS;
    std::vector<MeshRef<SLICES_N>> slices;

    for (const auto &sg_gens : Combos(g_gens, SLICES_N - 1)) {
        const auto s = triangulate<SLICES_N>(group, sg_gens).tile(group, g_gens, sg_gens);

        slices.emplace_back(s);
    }

    //endregion

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * points.size(), &points[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint ubo;
    glGenBuffers(1, &ubo);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vbo);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo);

    glBindVertexArray(0);

    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto st = (float) glfwGetTime() / 8;
        Matrices mats = build(window, st);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(mats), &mats, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        const auto wires_dark = glm::vec3(.3, .3,.3);
        const auto wires_light = wires_dark;
        glBindProgramPipeline(proj_pipe);
        for (int i = 0; i < wires.size(); i++) {
            const auto &ref = wires[i];

            const float f = factor(i, slices.size());
            glm::vec3 c = glm::mix(wires_dark, wires_light, f);
            glProgramUniform3f(solid, 2, c.r, c.g, c.b);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ref.ibo);
            glDrawElements(WIRE_MODE, ref.index_count, GL_UNSIGNED_INT, nullptr);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

        const auto slice_dark = glm::vec3(.5, .3, .7);
        const auto slice_light = glm::vec3(.9, .9, .95);
        glBindProgramPipeline(slice_pipe);
        for (int i = 0; i < slices.size(); i++) {
            const auto &ref = slices[i];

            const float f = factor(i, slices.size());
            glm::vec3 c = glm::mix(slice_dark, slice_light, f);
            glProgramUniform3f(solid, 2, c.r, c.g, c.b);

            glBindVertexArray(ref.vao);
            glDrawArrays(SLICES_MODE, 0, ref.primitive_count);
        }

        glBindProgramPipeline(0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}
