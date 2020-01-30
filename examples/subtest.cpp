#include <iostream>
#include <vector>

#include <combo_iterator.hpp>
#include <numeric>

template<class T>
std::ostream &operator<<(std::ostream &o, const std::vector<T> &v) {
    for (const auto &e : v) o << e << " ";
    return o;
}

int main() {
    std::vector<int> gens(5);
    std::iota(gens.begin(), gens.end(), 0);

    const Combos<int> &combos = Combos(gens, 2);

    for (const auto &e : combos) {
        std::cout << e << std::endl;
    }

    return EXIT_SUCCESS;
}
