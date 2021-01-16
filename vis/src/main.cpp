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

mat5 wander(float time) {
    mat5 r = mat5::Identity();
    r *= rot<5>(0, 2, time * .15f);
    r *= rot<5>(1, 2, time * .13f);
    r *= rot<5>(0, 1, time * .20f);

    r *= rot<5>(0, 3, time * .17f);
    r *= rot<5>(1, 3, time * .25f);
    r *= rot<5>(2, 3, time * .12f);

//    r *= rot<5>(1, 4, time * .27f);

    return r;
}

class ExampleApplication : public nanogui::Screen {
public:
    std::unique_ptr<SliceRenderer<4>> ren;
    std::unique_ptr<cgl::Buffer<Matrices>> ubo;

    std::vector<Slice<4>> slices;

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

        auto *window = new Window(this, "Sample Window");
        window->setPosition(Vector2i(15, 15));
        window->setFixedWidth(250);
        window->setLayout(new BoxLayout(Orientation::Vertical));

//        auto pause = new ToolButton(window, ENTYPO_ICON_CONTROLLER_PAUS);
//        pause->setFlags(Button::ToggleButton);
//        pause->setChangeCallback([&](bool value) { this->paused = value; });

        performLayout();

        std::cout << utilInfo();

        {
            std::vector<int> symbol = {3, 4, 3, 2};
            auto group = tc::schlafli(symbol);
            auto ctx = generators(group);
            auto selected_ctxs = set_difference(
                combinations(ctx, 3),
                {
                    {0, 1, 2},
                }
            );
            auto mesh = fill_each_tile_merge<4>(group, selected_ctxs);

            auto &slice = slices.emplace_back(group);
            slice.setMesh(mesh);
            slice.root << .80, .02, .02, .02, .02;
        }

        ren = std::make_unique<SliceRenderer<4>>();

        ubo = std::make_unique<cgl::Buffer<Matrices>>();
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

        auto rotation = wander(time);
        for (auto &slice : slices) {
            slice.transform = rotation;
            slice.setPoints();
        }

        Matrices mats = Matrices::build(*this);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, *ubo);
        ubo->put(mats);
        for (const auto &slice : slices) {
            ren->draw(slice);
        }
    }
};

int main(int argc, char **argv) {
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
