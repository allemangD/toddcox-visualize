/*
    src/example4.cpp -- C++ version of an example application that shows
    how to use the OpenGL widget. For a Python implementation, see
    '../python/example4.py'.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/opengl.h>
#include <nanogui/nanogui.h>
#include <nanogui/glutil.h>

#include <iostream>
#include <string>

#include <geometry.hpp>
#include <solver.hpp>
#include <rendering.hpp>
#include <mirror.hpp>
#include <util.hpp>
#include <tc/groups.hpp>

struct Matrices {
    mat4 proj = mat4::Identity();
    mat4 view = mat4::Identity();

    Matrices() = default;

    Matrices(mat4 proj, mat4 view) : proj(std::move(proj)), view(std::move(view)) {}

    static Matrices build(const nanogui::Screen &screen) {
        auto aspect = (float) screen.width() / (float) screen.height();
        auto pheight = 1.4f;
        auto pwidth = aspect * pheight;
        mat4 proj = ortho(-pwidth, pwidth, -pheight, pheight, -10.0f, 10.0f);

//        if (!glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
//            state.st += state.time_delta / 8;
//        }

        auto view = mat4::Identity();
        return Matrices(proj, view);
    }
};

template<class C>
std::vector<vec4> points(const tc::Group &group, const C &coords, const float time) {
    auto cosets = group.solve();
    auto mirrors = mirror<5>(group);

    auto corners = plane_intersections(mirrors);
    auto start = barycentric(corners, coords);

    auto higher = cosets.path.walk<vec5, vec5>(start, mirrors, reflect<vec5>);

    mat5 r = mat5::Identity();
    r *= rot<5>(0, 2, time * .21f);
//    r *= rot<5>(1, 4, time * .27f);

    r *= rot<5>(0, 3, time * .17f);
    r *= rot<5>(1, 3, time * .25f);
    r *= rot<5>(2, 3, time * .12f);

    std::transform(higher.begin(), higher.end(), higher.begin(), [&](vec5 v) { return r * v; });

    std::vector<vec4> lower(higher.size());
    std::transform(higher.begin(), higher.end(), lower.begin(), stereo<4>);
    return lower;
}

template<int N, class T, class C>
Prop<4, vec4> make_slice(
    const tc::Group &g,
    const C &coords,
    vec3 color,
    T all_sg_gens,
    const std::vector<std::vector<int>> &exclude
) {
    Prop<N, vec4> res{};

//    res.vbo.put(points(g, coords));
    res.ibo.put(merge<N>(hull<N>(g, all_sg_gens, exclude)));
    res.vao.ipointer(0, res.ibo, 4, GL_UNSIGNED_INT);

    return res;
}

class ExampleApplication : public nanogui::Screen {
public:
    vec5 root;
    std::unique_ptr<tc::Group> group;
    std::unique_ptr<Prop<4, vec4>> prop;
    std::unique_ptr<cgl::Buffer<Matrices>> ubo;
    std::unique_ptr<SliceRenderer<4, vec4>> ren;

    float glfw_time = 0;
    float last_frame = 0;
    float frame_time = 0;
    float time = 0;

    bool paused = false;

    ExampleApplication() : nanogui::Screen(
        Eigen::Vector2i(1920, 1080),
        "Coset Visualization",
        true, false,
        8, 8, 24, 8,
        4,
        4, 5) {
        using namespace nanogui;

        Window *window = new Window(this, "Sample Window");
        window->setPosition(Vector2i(15, 15));
        window->setFixedWidth(250);
        window->setLayout(new BoxLayout(Orientation::Vertical));

        auto pause = new ToolButton(window, ENTYPO_ICON_CONTROLLER_PAUS);
        pause->setFlags(Button::ToggleButton);
        pause->setChangeCallback([&](bool value) { this->paused = value; });

        performLayout();

        std::cout << utilInfo();

        std::vector<int> symbol = {5, 3, 3, 2};
        root << .80, .02, .02, .02, .02;

        group = std::make_unique<tc::Group>(tc::schlafli(symbol));
        auto gens = generators(*group);
        std::vector<std::vector<int>> exclude = {{0, 1, 2}};
        auto combos = Combos<int>(gens, 3);

        prop = std::make_unique<Prop<4, vec4>>(make_slice<4>(*group, root, {}, combos, exclude));

        ubo = std::make_unique<cgl::Buffer<Matrices>>();
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, *ubo);

        ren = std::make_unique<SliceRenderer<4, vec4>>();
    }

    void drawContents() override {
        glEnable(GL_DEPTH_TEST);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        glViewport(0, 0, width(), height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfw_time = (float) glfwGetTime();
        frame_time = glfw_time - last_frame;
        last_frame = glfw_time;
        if (!paused) time += frame_time;

        std::get<0>(prop->vbos).put(points(*group, root, time));

        Matrices mats = Matrices::build(*this);
        ubo->put(mats);
        ren->draw(*prop);
    }
};

int main(int /* argc */, char ** /* argv */) {
    try {
        nanogui::init();

        /* scoped variables */ {
            nanogui::ref<ExampleApplication> app = new ExampleApplication();
            app->drawAll();
            app->setVisible(true);
            nanogui::mainloop(1);
        }

        nanogui::shutdown();
    } catch (const std::runtime_error &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        std::cerr << error_msg << std::endl;
        return -1;
    }

    return 0;
}