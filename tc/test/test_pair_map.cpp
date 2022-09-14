#include <functional>
#include <iostream>
#include <unordered_map>

#include <tc/pair_map.hpp>

int test_populate() {
    auto f = [](size_t i, size_t j) { return ((i + j) << 12) ^ i ^ j; };

    tc::pair_map<size_t> pm(6);

    for (int i = 0; i < pm.size(); ++i) {
        for (int j = i; j < pm.size(); ++j) {
            pm(i, j) = f(i, j);
        }
    }

    for (int i = 0; i < pm.size(); ++i) {
        for (int j = 0; j < pm.size(); ++j) {
            if (pm(i, j) != (f(i, j))) {
                std::cerr << "pm(" << i << ", " << j << ") != " << (f(i, j)) << "" << std::endl;
                return EXIT_FAILURE;
            }
            if (pm(j, i) != (f(i, j))) {
                std::cerr << "pm(" << j << ", " << i << ") != " << (f(i, j)) << "" << std::endl;
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}

int test_symmetry() {
    size_t key = 1;
    tc::pair_map<size_t> pm(6);

    for (int i = 0; i < pm.size(); ++i) {
        for (int j = i; j < pm.size(); ++j) {
            pm(i, j) = key++;
        }
    }

    for (int i = 0; i < pm.size(); ++i) {
        for (int j = 0; j < pm.size(); ++j) {
            if (pm(i, j) != pm(j, i)) {
                std::cerr << "pm(" << i << ", " << j << ") != pm(" << j << ", " << i << ")" << std::endl;
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}

int test_fill() {
    tc::pair_map<size_t> pm(6, 42);

    for (int i = 0; i < pm.size(); ++i) {
        for (int j = i; j < pm.size(); ++j) {
            if (pm(i, j) != 42) {
                std::cerr << "pm(" << i << ", " << j << ") != 42" << std::endl;
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}

int test_copy() {
    auto f = [](size_t i, size_t j) { return ((i + j) << 12) ^ i ^ j; };

    tc::pair_map<size_t> pm(6);

    for (int i = 0; i < pm.size(); ++i) {
        for (int j = i; j < pm.size(); ++j) {
            pm(i, j) = f(i, j);
        }
    }

    tc::pair_map<size_t> cp = pm;

    for (int i = 0; i < pm.size(); ++i) {
        for (int j = 0; j < pm.size(); ++j) {
            if (cp(i, j) != pm(i, j)) {
                std::cerr << "cp(" << i << ", " << j << ") (" << cp(i, j) << ") != pm(" << i << ", " << j << ") ("
                          << pm(i, j) << ")" << std::endl;
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}

int test_move() {
    auto f = [](size_t i, size_t j) { return ((i + j) << 12) ^ i ^ j; };

    tc::pair_map<size_t> pm(6);

    for (int i = 0; i < pm.size(); ++i) {
        for (int j = i; j < pm.size(); ++j) {
            pm(i, j) = f(i, j);
        }
    }

    tc::pair_map<size_t> cp = std::move(pm);

    for (int i = 0; i < cp.size(); ++i) {
        for (int j = 0; j < cp.size(); ++j) {
            if (cp(i, j) != (f(i, j))) {
                std::cerr << "cp(" << i << ", " << j << ") != " << (f(i, j)) << "" << std::endl;
                return EXIT_FAILURE;
            }
            if (cp(j, i) != (f(i, j))) {
                std::cerr << "cp(" << j << ", " << i << ") != " << (f(i, j)) << "" << std::endl;
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}

int test_iterate() {
    auto f = [](size_t i, size_t j) { return ((i + j) << 12) ^ i ^ j; };

    tc::pair_map<size_t> pm(6);

    for (int i = 0; i < pm.size(); ++i) {
        for (int j = i; j < pm.size(); ++j) {
            pm(i, j) = f(i, j);
        }
    }

    size_t count = 0;
    for (const auto &[i, j, m]: pm) {
        if (m != f(i, j)) {
            std::cerr << "m (" << m << ") != " << (f(i, j)) << "" << std::endl;
            return EXIT_FAILURE;
        }
        count++;
    }

    if (count != 21) {
        std::cerr << "count (" << count << ") != " << 21 << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int test_iterate_ref() {
    auto f = [](size_t i, size_t j) { return ((i + j) << 12) ^ i ^ j; };

    tc::pair_map<size_t> pm(6);

    for (const auto &[i, j, m]: pm) {
        m = f(i, j);
    }

    for (const auto &[i, j, m]: pm) {
        if (m != f(i, j)) {
            std::cerr << "m (" << m << ") != " << f(i, j) << std::endl;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

int test_view() {
    auto f = [](size_t i, size_t j) { return ((i + j) << 12) ^ i ^ j; };

    tc::pair_map<size_t> pm(6);

    for (int i = 0; i < pm.size(); ++i) {
        for (int j = i; j < pm.size(); ++j) {
            pm(i, j) = f(i, j);
        }
    }

    size_t count = 0;
    for (const auto &[i, j, m]: pm.of(4)) {
        if (i != 4 && j != 4) {
            std::cerr << i << ", " << j << " != " << 4 << std::endl;
            return EXIT_FAILURE;
        }
        count++;
    }

    if (count != 6) {
        std::cerr << "count (" << count << ") != " << 6 << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static std::unordered_map<std::string, std::function<int()>> tests = {
    {"populate",    test_populate},
    {"symmetry",    test_symmetry},
    {"fill",        test_fill},
    {"copy",        test_copy},
    {"move",        test_move},
    {"iterate",     test_iterate},
    {"iterate_ref", test_iterate_ref},
    {"view",        test_view},
};

int main(int argc, char *argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);

    auto it = tests.find(args[0]);
    if (it == tests.end()) {
        std::cerr << "Test not found" << std::endl;
        return EXIT_FAILURE;
    }
    auto test = it->second;
    return test();
}
