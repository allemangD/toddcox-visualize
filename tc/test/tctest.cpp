#include <iostream>
#include <string>
#include <vector>

#include <tc/groups.hpp>
#include <tc/core.hpp>

int main(int argc, char *argv[]) {
    std::string key = argv[1];

    std::vector<std::pair<tc::Group, size_t>> groups;

    // See the group orders here https://en.wikipedia.org/wiki/Coxeter_group#Properties
    if (key == "A") {
        groups = {
            {tc::group::A(1), 2},
            {tc::group::A(2), 6},
            {tc::group::A(3), 24},
            {tc::group::A(4), 120},
        };
    }
    if (key == "B") {
        groups = {
            {tc::group::B(2), 8},
            {tc::group::B(3), 48},
            {tc::group::B(4), 384},
            {tc::group::B(5), 3840},
            {tc::group::B(6), 46080},
        };
    }
    if (key == "D") {
        groups = {
            {tc::group::D(2), 4},
            {tc::group::D(3), 24},
            {tc::group::D(4), 192},
            {tc::group::D(5), 1920},
            {tc::group::D(6), 23040},
        };
    }
    if (key == "E") {
        groups = {
            {tc::group::E(3), 12},
            {tc::group::E(4), 120},
            {tc::group::E(5), 1920},
            {tc::group::E(6), 51840},
        };
    }
    if (key == "F") {
        groups = {
            {tc::group::F4(), 1152},
        };
    }
    if (key == "G") {
        groups = {
            {tc::group::G2(), 12},
        };
    }
    if (key == "H") {
        groups = {
            {tc::group::H(2), 10},
            {tc::group::H(3), 120},
            {tc::group::H(4), 14400},
        };
    }
    if (key == "I") {
        groups = {
            {tc::group::I2(2), 4},
            {tc::group::I2(3), 6},
            {tc::group::I2(4), 8},
            {tc::group::I2(5), 10},
        };
    }
    if (key == "T") {
        groups = {
            {tc::group::T(3),        36},
            {tc::group::T(4),        64},
            {tc::group::T(400),      640000},
            {tc::group::T(400, 300), 480000},
        };
    }

    int status = EXIT_SUCCESS;

    for (const auto &[group, expected]: groups) {
        auto cos = group.solve({});
        auto actual = cos.size();
        std::cout << group.name << " : " << actual;
        if (expected != actual) {
            std::cout << " (Expected " << expected << ")";
            status = EXIT_FAILURE;
        }
        std::cout << std::endl;
    }

    return status;
}
