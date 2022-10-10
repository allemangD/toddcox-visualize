#pragma once

#include <gl/debug.hpp>
#include <gl/buffer.hpp>
#include <gl/shader.hpp>
#include <gl/vertexarray.hpp>

#include <fstream>

struct State {
    Eigen::Vector4f bg{0.16, 0.16, 0.16, 1.00};
    Eigen::Vector4f fg{0.71, 0.53, 0.94, 1.00};

    float time = 0;

    Eigen::Projective3f proj = Eigen::Projective3f::Identity();
    Eigen::Projective3f view = Eigen::Projective3f::Identity();
};

template<typename V_=Eigen::Vector4f>
struct PointRenderer {
    using Vertex = V_;

    VertexShader vs{std::ifstream("res/shaders/main.vert.glsl")};
    FragmentShader fs{std::ifstream("res/shaders/main.frag.glsl")};

    Program pgm{vs, fs};

    Buffer<Vertex> vbo;
    VertexArray<Vertex> vao{vbo};

    GLuint count;

    template<typename T>
    void upload(const T &points) {
        count = vbo.upload(points);

//    Buffer<GLuint> ind_buf;
//    glVertexArrayElementBuffer(vao, ind_buf);
    }

    void draw(const State &state) {
        glUseProgram(pgm);
        glBindVertexArray(vao);
        glUniform4fv(0, 1, state.fg.data());
        glUniform1f(1, state.time);
        glUniformMatrix4fv(2, 1, false, state.proj.data());
        glUniformMatrix4fv(3, 1, false, state.view.data());
        glDrawArrays(GL_POINTS, 0, count);
        glBindVertexArray(0);
        glUseProgram(0);
    }
};
