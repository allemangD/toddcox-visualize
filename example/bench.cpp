#include "solver.h"
#include "groups.h"

#include <ctime>
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
        auto s = std::clock(); // to measure CPU time
        auto cosets = solve(group);
        auto e = std::clock();

        double diff = (double) (e - s) / CLOCKS_PER_SEC;
        int order = cosets.len;

        std::cout << group.name << "," << order << "," << diff << std::endl;
    }

    return 0;
}
