#include <iostream>
#include <string>
#include <vector>

#include <tc/groups.hpp>
#include <tc/core.hpp>

int main(int argc, char *argv[]) {
    std::string key = argv[1];

    std::cerr << "Min. cos/s: " << MINIMUM_COS_PER_SEC << std::endl;
    std::vector<std::tuple<std::string, tc::Group, std::vector<unsigned int>, size_t>> groups;

    // See the group orders here https://en.wikipedia.org/wiki/Coxeter_group#Properties
    if (key == "B") {
        groups = {
            {"B(7)", tc::group::B(7), {}, 645120},
            {"B(8)", tc::group::B(8), {}, 10321920},
        };
    }
    if (key == "E") {
        groups = {
            {"E(6)", tc::group::E(6), {}, 51840},
            {"E(7)", tc::group::E(7), {}, 2903040},
        };
    }
    if (key == "T") {
        groups = {
            {"T(500)",  tc::group::T(500),  {}, 1000000},
            {"T(1000)", tc::group::T(1000), {}, 4000000},
        };
    }

    int status = EXIT_SUCCESS;

    for (const auto &[name, group, sub_gens, expected]: groups) {
        auto t0 = clock();
        auto cos = tc::solve(group, sub_gens);
        auto t1 = clock();
        auto actual = cos.size();

        auto sec = (double) (t1 - t0) / CLOCKS_PER_SEC;
        auto cos_per_sec = (double) actual / sec;

        if (expected != actual) {
            std::cerr << name << " wrong. " << actual << " (" << expected << ")" << std::endl;
            status = EXIT_FAILURE;
        }

        std::cout << name << " cos/s: " << (size_t) cos_per_sec << std::endl;
        if (cos_per_sec < MINIMUM_COS_PER_SEC) {
            std::cerr << name << " too slow." << std::endl;
            status = EXIT_FAILURE;
        }
    }

    return status;
}
