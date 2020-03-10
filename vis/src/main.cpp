#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>

#include <tc/groups.hpp>

#include "util.hpp"
#include "mirror.hpp"
#include "geometry.hpp"

#include <cgl/render.hpp>

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
struct DirectMesh {
    cgl::buffer<Primitive<N>> ibo;
    GLenum mode;

    explicit DirectMesh(GLenum mode, const Mesh<N> &mesh)
        : ibo(), mode(mode) {

        ibo.put(mesh.prims);
    }

    void draw() const {
        ibo.bound(GL_ELEMENT_ARRAY_BUFFER, [&]() {
            glDrawElements(mode, ibo.count() * N, GL_UNSIGNED_INT, nullptr);
        });
    }
};

template<unsigned N>
struct DeferredMesh {
    cgl::vertexarray vao;
    cgl::buffer<Primitive<N>> ibo;

    explicit DeferredMesh(const Mesh<N> &mesh, const cgl::buffer<glm::vec4> &color)
        : ibo(), vao() {

        ibo.put(mesh.prims);

        vao.bound([&]() {
            ibo.bound(GL_ARRAY_BUFFER, [&]() {
                glEnableVertexAttribArray(0);
                glVertexAttribIPointer(0, N, GL_INT, 0, nullptr);
            });
            color.bound(GL_ARRAY_BUFFER, [&]() {
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
            });
        });
    }

    void draw() const {
        vao.bound([&]() {
            glDrawArrays(GL_POINTS, 0, ibo.count() * N);
        });
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

template<unsigned N>
std::vector<Mesh<N>> poly_parts(const tc::Group &group) {
    std::vector<Mesh<N>> parts;
    auto g_gens = gens(group);
    for (const auto &sg_gens : Combos(g_gens, N - 1)) {
        parts.push_back(
            triangulate<N>(group, sg_gens).tile(group, g_gens, sg_gens)
        );
    }
    return parts;
}

void run(GLFWwindow *window) {
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    auto defer = cgl::pgm::vert::file("shaders/slice/deferred.vs.glsl");
    auto direct_ortho = cgl::pgm::vert::file("shaders/direct-ortho.vs.glsl");
    auto direct_stereo = cgl::pgm::vert::file("shaders/direct-stereo.vs.glsl");

    auto slice = cgl::pgm::geom::file("shaders/slice/slice.gm.glsl");
    auto curve_stereo = cgl::pgm::geom::file("shaders/curve-stereo.gm.glsl");
    auto curve_ortho = cgl::pgm::geom::file("shaders/curve-ortho.gm.glsl");

    auto solid = cgl::pgm::frag::file("shaders/solid.fs.glsl");
    auto diffuse = cgl::pgm::frag::file("shaders/diffuse.fs.glsl");

    auto proj_pipe = cgl::pipeline();
    proj_pipe
        .stage(direct_stereo)
        .stage(solid);

    auto slice_pipe = cgl::pipeline();
    slice_pipe
        .stage(defer)
        .stage(slice)
        .stage(diffuse);

    //region points
    auto group = tc::group::F4();
    auto res = group.solve();
    auto mirrors = mirror(group);

    auto corners = plane_intersections(mirrors);
//    auto start = barycentric(corners, {1.0f, 1.0f, 1.0f, 1.0f});
    auto start = barycentric(corners, {1.00f, 0.2f, 0.1f, 0.05f});
//    auto start = barycentric(corners, {0.05f, 0.1f, 0.2f, 1.00f});
    auto points = res.path.walk<glm::vec4, glm::vec4>(start, mirrors, reflect);

    auto g_gens = gens(group);

    auto wire_data = merge(poly_parts<2>(group));
    DirectMesh<2> wires(GL_LINES, wire_data);

    const auto slice_dark = glm::vec3(.5, .3, .7);
    const auto slice_light = glm::vec3(.9, .9, .95);

    const auto slice_parts = poly_parts<4>(group);
    auto slice_data = merge(slice_parts);
    auto slice_colors = std::vector<glm::vec4>(slice_data.size());
    for (int i = 0, k = 0; i < slice_parts.size(); ++i) {
        auto fac = factor(i, slice_parts.size());
        glm::vec3 color = glm::mix(slice_dark, slice_light, fac);

        for (int j = 0; j < slice_parts[i].size(); ++j, ++k) {
            slice_colors[k] = glm::vec4(color, 1);
        }
    }
    cgl::buffer<glm::vec4> slice_colors_buf(slice_colors);
    DeferredMesh<4> slices(slice_data, slice_colors_buf);

    //endregion

    cgl::buffer<glm::vec4> vbo(points);

    cgl::buffer<Matrices> ubo;

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vbo);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo);

    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto st = (float) glfwGetTime() / 8;
        Matrices mats = build(window, st);
        ubo.put(mats);

        glLineWidth(1.5);

        glProgramUniform3f(solid, 2, 0.3, 0.3, 0.3);
        proj_pipe.bound([&]() {
            wires.draw();
        });

        slice_pipe.bound([&]() {
            slices.draw();
        });

        glfwSwapInterval(2);
        glfwSwapBuffers(window);

        glfwPollEvents();
    }
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
    glfwSwapInterval(1);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);
    //endregion

    std::cout << utilInfo();

    run(window);

    glfwTerminate();
    return EXIT_SUCCESS;
}
