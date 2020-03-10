#pragma once

#include <stdexcept>

#include <glad/glad.h>

namespace cgl {
    class gl_error : public std::domain_error {
    public:
        explicit gl_error(const std::string &arg) : domain_error(arg) {}

        explicit gl_error(const char *string) : domain_error(string) {}
    };

    class shader_error : public gl_error {
    public:
        explicit shader_error(const std::string &arg) : gl_error(arg) {}

        explicit shader_error(const char *string) : gl_error(string) {}
    };

    class program_error : public gl_error {
    public:
        explicit program_error(const std::string &arg) : gl_error(arg) {}

        explicit program_error(const char *string) : gl_error(string) {}
    };
}