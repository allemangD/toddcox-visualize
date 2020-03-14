#include <combo_iterator.hpp>
#include <iostream>

int main() {
    auto cs = Combos<int>({7, 2, 3}, 2);

    auto beg = cs.begin();
    auto end = cs.end();

    while (beg != end) {
        const auto &c = *(++beg);
        for (const auto &e : c) std::cout << e << " ";
        std::cout << std::endl;
    }
}
