#include <iostream>
#include <string>
#include <vector>

#include <tc/groups.hpp>
#include <tc/core.hpp>

#define UNWRAP(...) {__VA_ARGS__}
#define CASE(GROUP, SUBGROUP, EXPECT) {#GROUP " / " #SUBGROUP, UNWRAP SUBGROUP, SUBGROUP, EXPECT}
//#define CASE(GROUP, SUBGROUP, EXPECT) {#GROUP " / " #SUBGROUP, tc::group::GROUP, SUBGROUP, EXPECT}


int main(int argc, char *argv[]) {
    std::string key = argv[1];

    std::vector<std::tuple<
        std::string,
        tc::Group,
        std::vector<tc::Gen>,
        size_t
    >> groups;

    // See the group orders here https://en.wikipedia.org/wiki/Coxeter_group#Properties
    if (key == "A") {
        groups = {
            {"A(1)", tc::group::A(1), {},     2},
            {"A(2)", tc::group::A(2), {},     6},
            {"A(3)", tc::group::A(3), {},     24},
            {"A(3)", tc::group::A(3), {0},    12},
            {"A(3)", tc::group::A(3), {0, 1}, 4},
            {"A(3)", tc::group::A(3), {0, 2}, 6},
            {"A(3)", tc::group::A(3), {2},    12},
            {"A(4)", tc::group::A(4), {},     120},
            {"A(4)", tc::group::A(4), {0},    60},
            {"A(4)", tc::group::A(4), {0, 1}, 20},
            {"A(4)", tc::group::A(4), {2},    60},
            {"A(4)", tc::group::A(4), {0, 2}, 30},
        };
    }
    if (key == "B") {
        groups = {
            {"B(2)", tc::group::B(2), {},           8},
            {"B(3)", tc::group::B(3), {},           48},
            {"B(3)", tc::group::B(3), {0},          24},
            {"B(3)", tc::group::B(3), {0, 2},       12},
            {"B(4)", tc::group::B(4), {},           384},
            {"B(4)", tc::group::B(4), {0},          192},
            {"B(4)", tc::group::B(4), {0, 2},       96},
            {"B(5)", tc::group::B(5), {},           3840},
            {"B(5)", tc::group::B(5), {0},          1920},
            {"B(5)", tc::group::B(5), {0, 2},       960},
            {"B(5)", tc::group::B(5), {0, 2, 3},    320},
            {"B(6)", tc::group::B(6), {},           46080},
            {"B(6)", tc::group::B(6), {0},          23040},
            {"B(6)", tc::group::B(6), {0, 2},       11520},
            {"B(6)", tc::group::B(6), {0, 2, 3},    3840},
            {"B(6)", tc::group::B(6), {0, 2, 3, 5}, 1920},
        };
    }
    if (key == "D") {
        groups = {
            {"D(2)", tc::group::D(2), {},           4},
            {"D(3)", tc::group::D(3), {},           24},
            {"D(4)", tc::group::D(4), {},           192},
            {"D(4)", tc::group::D(4), {0, 1},       32},
            {"D(4)", tc::group::D(4), {0, 1, 3},    8},
            {"D(5)", tc::group::D(5), {},           1920},
            {"D(5)", tc::group::D(5), {0, 1},       320},
            {"D(5)", tc::group::D(5), {0, 1, 3},    160},
            {"D(5)", tc::group::D(5), {0, 1, 3, 4}, 40},
            {"D(6)", tc::group::D(6), {},           23040},
            {"D(6)", tc::group::D(6), {0, 1},       3840},
            {"D(6)", tc::group::D(6), {0, 1, 3},    1920},
            {"D(6)", tc::group::D(6), {0, 1, 3, 5}, 480},
        };
    }
    if (key == "E") {
        groups = {
            {"E(3)", tc::group::E(3), {},        12},
            {"E(4)", tc::group::E(4), {},        120},
            {"E(4)", tc::group::E(4), {2},       60},
            {"E(4)", tc::group::E(4), {2, 1},    20},
            {"E(4)", tc::group::E(4), {2, 1, 3}, 5},
            {"E(5)", tc::group::E(5), {},        1920},
            {"E(5)", tc::group::E(5), {2},       960},
            {"E(5)", tc::group::E(5), {2, 1},    320},
            {"E(5)", tc::group::E(5), {2, 1, 3}, 80},
            {"E(6)", tc::group::E(6), {},        51840},
            {"E(6)", tc::group::E(6), {2},       25920},
            {"E(6)", tc::group::E(6), {2, 1},    8640},
            {"E(6)", tc::group::E(6), {2, 1, 3}, 2160},
        };
    }
    if (key == "F") {
        groups = {
            {"F4()", tc::group::F4(), {},        1152},
            {"F4()", tc::group::F4(), {0},       576},
            {"F4()", tc::group::F4(), {0, 2},    288},
            {"F4()", tc::group::F4(), {1, 3},    288},
            {"F4()", tc::group::F4(), {1, 2, 3}, 24},
        };
    }
    if (key == "G") {
        groups = {
            {"G2()", tc::group::G2(), {},  12},
            {"G2()", tc::group::G2(), {0}, 6},
            {"G2()", tc::group::G2(), {1}, 6},
        };
    }
    if (key == "H") {
        groups = {
            {"H(2)", tc::group::H(2), {},     10},
            {"H(2)", tc::group::H(2), {0},    5},
            {"H(2)", tc::group::H(2), {1},    5},
            {"H(3)", tc::group::H(3), {},     120},
            {"H(3)", tc::group::H(3), {0},    60},
            {"H(3)",   tc::group::H(3), {0, 1}, 12},
            {"H(3)",   tc::group::H(3), {0, 2}, 30},
            {"H(3)",   tc::group::H(3), {1, 2}, 20},
            {"H(4)", tc::group::H(4), {},     14400},
            {"H(4)", tc::group::H(4), {0},    7200},
            {"H(4)", tc::group::H(4), {1},    7200},
            {"H(4)", tc::group::H(4), {1, 3}, 3600},
        };
    }
    if (key == "I") {
        groups = {
            {"I2(2)", tc::group::I2(2), {},  4},
            {"I2(3)", tc::group::I2(3), {},  6},
            {"I2(3)", tc::group::I2(3), {0}, 3},
            {"I2(3)", tc::group::I2(3), {1}, 3},
            {"I2(4)", tc::group::I2(4), {},  8},
            {"I2(4)", tc::group::I2(4), {0}, 4},
            {"I2(4)", tc::group::I2(4), {1}, 4},
            {"I2(5)", tc::group::I2(5), {},  10},
            {"I2(5)", tc::group::I2(5), {0}, 5},
            {"I2(5)", tc::group::I2(5), {1}, 5},
        };
    }
    if (key == "T") {
        groups = {
            {"T(3)",        tc::group::T(3),        {},     36},
            {"T(4)",        tc::group::T(4),        {},     64},
            {"T(400)",      tc::group::T(400),      {},     640000},
            {"T(400)",      tc::group::T(400),      {0},    320000},
            {"T(400)",      tc::group::T(400),      {0, 2}, 160000},
            {"T(400, 300)", tc::group::T(400, 300), {},     480000},
            {"T(400, 300)", tc::group::T(400, 300), {0},    240000},
            {"T(400, 300)", tc::group::T(400, 300), {0, 2}, 120000},
        };
    }

    int status = EXIT_SUCCESS;

    for (const auto &[name, group, sub_gens, expected]: groups) {
        auto cos = tc::solve(group, sub_gens);
        auto actual = cos.size();
        std::cout << name;
        if (!sub_gens.empty()) {
            std::cout << " / {";
            std::cout << (int) sub_gens[0];
            for (int i = 1; i < sub_gens.size(); ++i) {
                std::cout << ", " << (int) sub_gens[i];
            }
            std::cout << "}";
        }
        std::cout << " : " << actual;
        if (expected != actual) {
            std::cout << " (Expected " << expected << ")";
            status = EXIT_FAILURE;
        }
        std::cout << std::endl;
    }

    return status;
}
