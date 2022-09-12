#include <functional>
#include <iostream>
#include <unordered_map>

#include <tc/pair_map.hpp>

int iterate_ref() {
    tc::pair_map<size_t> pm(4, 2);

    for (auto [i, j, m]: pm) {
        m = i + j;
    }

    for (auto [i, j, m]: pm) {
        std::cout << "(" << i << "," << j << ") = " << m << std::endl;
    }

    return EXIT_SUCCESS;
}

int iterate_const() {
    tc::pair_map<size_t> pm(4, 2);
    for (const auto &[i, j, m]: pm) {
        m = i + j;
    }

    const tc::pair_map<size_t> pmc = pm;
    for (auto [i, j, m]: pmc) {
        std::cout << "(" << i << "," << j << ") = " << m << std::endl;
    }

    return EXIT_SUCCESS;
}

int iterate() {
    tc::pair_map<size_t> pm(4, 2);

    std::cout << pm(0, 0) << std::endl;
    pm(0, 0) = 3;
    std::cout << pm(0, 0) << std::endl;

    for (auto [i, j, m]: pm) {
        std::cout << i << " " << j << " = " << m << std::endl;
    }

    std::cout << pm(1, 0) << " " << pm(0, 1) << std::endl;
    pm(1, 0) = 7;
    std::cout << pm(1, 0) << " " << pm(0, 1) << std::endl;
    pm(0, 1) = 9;
    std::cout << pm(1, 0) << " " << pm(0, 1) << std::endl;

    return EXIT_SUCCESS;
}

int view() {
    tc::pair_map<size_t> pm(4, 2);

    for (int i = 0; i < 4; ++i) {
        for (int j = i; j < 4; ++j) {
            pm(i, j) = i + j;
        }
    }

    std::cout << "ALL:" << std::endl;
    for (auto [i, j, m]: pm) {
        std::cout << "  (" << i << "," << j << ") = " << m << std::endl;
    }

    std::cout << "VIEW:" << std::endl;
    for (auto [i, j, m]: pm.of(2)) {
        std::cout << "  (" << i << "," << j << ") = " << m << std::endl;
    }

    return EXIT_SUCCESS;
}

static std::unordered_map<std::string, std::function<int()>> tests = {
    {"iterate_ref",   iterate_ref},
    {"iterate_const", iterate_const},
    {"iterate",       iterate},
    {"view",          view},
};

int main(int argc, char *argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);

    auto it = tests.find(args[0]);
    if (it == tests.end()) {
        std::cout << "Test not found" << std::endl;
        return EXIT_FAILURE;
    }
    auto test = it->second;
    return test();
}
