#pragma once

#include <vector>

#include <tc/group.hpp>

namespace tc {
    /**
     * Construct a group from a (simplified) Schlafli Symbol of the form [a, b, ..., c]
     * @param mults: The sequence of multiplicites between adjacent generators.
     */
    Group schlafli(const std::vector<int> &mults);

    Group coxeter(const std::string &symbol);

    Group vcoxeter(const std::string &symbol, std::vector<int> &values);

    template<typename ...Args>
    Group coxeter(const std::string &symbol, const Args &... args) {
        std::vector<int> values = {{args...}};
        return vcoxeter(symbol, values);
    }
}
