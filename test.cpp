#include "groups.cpp"
#include <chrono>
#include <iostream>

int main() {
    Group g = B(9);

    auto s = std::chrono::system_clock::now();
    auto cosets = g.solve();
    auto e = std::chrono::system_clock::now();

    std::chrono::duration<double> diff = e - s;
    int order = cosets.len;

    std::cout << order << std::endl;
    std::cout << diff.count() << std::endl;

    return 0;
}
