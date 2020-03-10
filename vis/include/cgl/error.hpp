#pragma once

#include <stdexcept>

#include <glad/glad.h>

namespace cgl {
    class GlError : public std::domain_error {
    public:
        explicit GlError(const std::string &arg) : domain_error(arg) {}

        explicit GlError(const char *string) : domain_error(string) {}
    };

    class ShaderError : public GlError {
    public:
        explicit ShaderError(const std::string &arg) : GlError(arg) {}

        explicit ShaderError(const char *string) : GlError(string) {}
    };

    class ProgramError : public GlError {
    public:
        explicit ProgramError(const std::string &arg) : GlError(arg) {}

        explicit ProgramError(const char *string) : GlError(string) {}
    };
}