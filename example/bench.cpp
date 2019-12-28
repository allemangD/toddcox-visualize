#include "solver.h"
#include "groups.h"
#include <chrono>
#include <iostream>

int main() {
    std::vector<tc::Group> groups = {
        tc::group::H(2),
        tc::group::H(3),
        tc::group::H(4),
        tc::group::T(100),
        tc::group::T(500),
        tc::group::T(1000),
        tc::group::E(6),
        tc::group::E(7),
    };

    for (const auto &group : groups) {
        auto s = std::chrono::high_resolution_clock::now();
        auto cosets = solve(group);
        auto e = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> diff = e - s;
        int order = cosets.len;

        std::cout << group.name << "," << order << "," << diff.count() << std::endl;
    }

    return 0;
}
