#pragma once

#include <gl/debug.hpp>
#include <gl/buffer.hpp>
#include <gl/shader.hpp>
#include <gl/vertexarray.hpp>

#include <fstream>

#include <shaders.hpp>

struct State {
    Eigen::Vector4f bg{0.16, 0.16, 0.16, 1.00};
    Eigen::Vector4f fg{0.71, 0.53, 0.94, 1.00};

    float time = 0;

    Eigen::Projective3f proj = Eigen::Projective3f::Identity();
    Eigen::Projective3f view = Eigen::Projective3f::Identity();
};

template<typename V_=Eigen::Vector4f>
struct PointCloud {
    using Vertex = V_;

    Buffer<Vertex> vbo;
    VertexArray<Vertex> vao{vbo};

    GLuint count{};

    template<typename T>
    void upload(const T &points) {
        count = vbo.upload(points);

//    Buffer<GLuint> ind_buf;
//    glVertexArrayElementBuffer(vao, ind_buf);
    }
};

template<typename V_=Eigen::Vector4f>
struct PointRenderer {
    using Vertex = V_;

    VertexShader vs{shaders::main_vert_glsl};
    FragmentShader fs{shaders::main_frag_glsl};

    Program pgm{vs, fs};

    void draw(const PointCloud<Vertex> &obj, const State &state) {
        glUseProgram(pgm);
        glBindVertexArray(obj.vao);
        glUniform4fv(0, 1, state.fg.data());
        glUniform1f(1, state.time);
        glUniformMatrix4fv(2, 1, false, state.proj.data());
        glUniformMatrix4fv(3, 1, false, state.view.data());
        glDrawArrays(GL_POINTS, 0, obj.count);
        glBindVertexArray(0);
        glUseProgram(0);
    }
};

template<typename V_=Eigen::Vector4f>
struct LineCloud {
    using Vertex = V_;
    
    Buffer<Vertex> vbo{};
    Buffer<unsigned> ibo{};
    VertexArray<Vertex> vao{vbo};

    GLuint count{};

    LineCloud() {
        glVertexArrayElementBuffer(vao, ibo);
    }

    template<typename T, typename U>
    void upload(const T &points, const U &inds) {
        vbo.upload(points);
        
        count = ibo.upload(inds);
    }
};

template<typename V_=Eigen::Vector4f>
struct LineRenderer {
    using Vertex = V_;

    VertexShader vs{std::ifstream("res/shaders/main.vert.glsl")};
    FragmentShader fs{std::ifstream("res/shaders/main.frag.glsl")};
    
    Program pgm{vs, fs};
    
    void draw(const LineCloud<Vertex> &obj, const State &state) {
        glUseProgram(pgm);
        glBindVertexArray(obj.vao);
        glUniform4fv(0, 1, state.fg.data());
        glUniform1f(1, state.time);
        glUniformMatrix4fv(2, 1, false, state.proj.data());
        glUniformMatrix4fv(3, 1, false, state.view.data());
        glDrawElements(GL_LINES, obj.count, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
        glUseProgram(0);
    }
};
