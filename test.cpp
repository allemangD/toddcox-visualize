#include "solver.h"
#include "groups.h"
#include <chrono>
#include <iostream>

int main() {
    tc::Group g = tc::group::T(2, 5000);

    auto s = std::chrono::system_clock::now();
    auto cosets = solve(g);
    auto e = std::chrono::system_clock::now();

    std::chrono::duration<double> diff = e - s;
    int order = cosets.len;

    std::cout << order << std::endl;
    std::cout << diff.count() << std::endl;

    return 0;
}
