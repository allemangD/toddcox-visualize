#include "solver.h"
#include "groups.h"

#include <ctime>
#include <iostream>

int main() {
    auto cube = tc::group::B(3);
    auto vars = tc::solve(cube, {});

    for (int target = 1; target < vars.len; target++) {
        auto &action = vars.path[target];
        std::cout << action.coset << " " << action.gen << " " << target << std::endl;
    }

    return 0;
}
