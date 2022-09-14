#include <iostream>
#include <string>
#include <vector>

#include <tc/groups.hpp>
#include <tc/core.hpp>

int main(int argc, char *argv[]) {
    std::string key = argv[1];

    std::vector<std::tuple<std::string, tc::Group, size_t>> groups;

    // See the group orders here https://en.wikipedia.org/wiki/Coxeter_group#Properties
    if (key == "A") {
        groups = {
            {"A(1)", tc::group::A(1), 2},
            {"A(2)", tc::group::A(2), 6},
            {"A(3)", tc::group::A(3), 24},
            {"A(4)", tc::group::A(4), 120},
        };
    }
    if (key == "B") {
        groups = {
            {"B(2)", tc::group::B(2), 8},
            {"B(3)", tc::group::B(3), 48},
            {"B(4)", tc::group::B(4), 384},
            {"B(5)", tc::group::B(5), 3840},
            {"B(6)", tc::group::B(6), 46080},
        };
    }
    if (key == "D") {
        groups = {
            {"D(2)", tc::group::D(2), 4},
            {"D(3)", tc::group::D(3), 24},
            {"D(4)", tc::group::D(4), 192},
            {"D(5)", tc::group::D(5), 1920},
            {"D(6)", tc::group::D(6), 23040},
        };
    }
    if (key == "E") {
        groups = {
            {"E(3)", tc::group::E(3), 12},
            {"E(4)", tc::group::E(4), 120},
            {"E(5)", tc::group::E(5), 1920},
            {"E(6)", tc::group::E(6), 51840},
        };
    }
    if (key == "F") {
        groups = {
            {"F4()", tc::group::F4(), 1152},
        };
    }
    if (key == "G") {
        groups = {
            {"G2()", tc::group::G2(), 12},
        };
    }
    if (key == "H") {
        groups = {
            {"H(2)", tc::group::H(2), 10},
            {"H(3)", tc::group::H(3), 120},
            {"H(4)", tc::group::H(4), 14400},
        };
    }
    if (key == "I") {
        groups = {
            {"I2(2)", tc::group::I2(2), 4},
            {"I2(3)", tc::group::I2(3), 6},
            {"I2(4)", tc::group::I2(4), 8},
            {"I2(5)", tc::group::I2(5), 10},
        };
    }
    if (key == "T") {
        groups = {
            {"T(3)", tc::group::T(3),        36},
            {"T(4)", tc::group::T(4),        64},
            {"T(400)", tc::group::T(400),      640000},
            {"T(400, 300)", tc::group::T(400, 300), 480000},
        };
    }

    int status = EXIT_SUCCESS;

    for (const auto &[name, group, expected]: groups) {
        auto cos = group.solve({});
        auto actual = cos.size();
        std::cout << name << " : " << actual;
        if (expected != actual) {
            std::cout << " (Expected " << expected << ")";
            status = EXIT_FAILURE;
        }
        std::cout << std::endl;
    }

    return status;
}
