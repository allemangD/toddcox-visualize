#include <combo.hpp>

#include <iostream>

std::ostream &operator<<(std::ostream &o, const std::vector<int> &data) {
    o << "[ ";
    for (const auto &el: data) {
        o << el << " ";
    }
    o << "]";
    return o;
}

int main() {
    std::vector<int> data{1, 2, 3, 4, 5};

    for (const auto &combo: combinations(data, 3)) {
        std::cout << combo << std::endl;
    }

    return EXIT_SUCCESS;
}
