#include <ctime>
#include <string>
#include <vector>

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <tc/core.hpp>
#include <tc/groups.hpp>

#define NAMED(x) #x, x

void bench(std::string group_expr, tc::Group group, const std::vector<tc::Gen> &gens) {
    std::clock_t s = std::clock();
    tc::Cosets cosets = tc::solve(group, gens);
    std::clock_t e = std::clock();

    double time = (double) (e - s) / CLOCKS_PER_SEC;
    tc::Coset order = cosets.size();
    size_t cos_s = (size_t) (order / time);

    std::string name = fmt::format("{}/{}", group_expr, gens);
    std::string row = fmt::format(
        "{:>24},{:>10},{:>8.3f}s,{:>10L}",
        name, order, time, cos_s
    );
    fmt::print("{}\n", row);
}

template<typename ...T>
tc::Group sch(T ...arg) {
    std::vector<int> mults{arg...,};
    return tc::schlafli(mults);
}

int main(int argc, char *argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);

    using namespace tc::group;

    fmt::print("{:>24},{:>10},{:>9},{:>10}\n", "NAME", "ORDER", "TIME", "COS/S");

    bench(NAMED(H(2)), {});
    bench(NAMED(H(3)), {});
    bench(NAMED(H(4)), {});
    bench(NAMED(sch(5, 3, 3, 2)), {});
    bench(NAMED(sch(5, 3, 3, 2)), {0});
    bench(NAMED(sch(5, 3, 3, 2)), {1});
    bench(NAMED(sch(5, 3, 3, 2)), {2});
    bench(NAMED(sch(5, 3, 3, 2)), {3});
    bench(NAMED(sch(5, 3, 3, 2)), {4});
    bench(NAMED(sch(5, 3, 3, 2)), {0, 1});
    bench(NAMED(sch(5, 3, 3, 2)), {0, 2});
    bench(NAMED(sch(5, 3, 3, 2)), {1, 2});
    bench(NAMED(sch(5, 3, 3, 2)), {0, 3});
    bench(NAMED(sch(5, 3, 3, 2)), {1, 3});
    bench(NAMED(sch(5, 3, 3, 2)), {2, 3});
    bench(NAMED(sch(5, 3, 3, 2)), {0, 4});
    bench(NAMED(sch(5, 3, 3, 2)), {1, 4});
    bench(NAMED(sch(5, 3, 3, 2)), {2, 4});
    bench(NAMED(sch(5, 3, 3, 2)), {3, 4});

    bench(NAMED(T(100)), {});
    bench(NAMED(T(500)), {});
    bench(NAMED(T(1000)), {});

    bench(NAMED(E(6)), {});
    bench(NAMED(E(7)), {});

    bench(NAMED(B(6)), {});
    bench(NAMED(B(7)), {});
    bench(NAMED(B(8)), {});

    return EXIT_SUCCESS;
}
