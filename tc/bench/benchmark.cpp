#include <ctime>
#include <string>
#include <vector>

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <tc/core.hpp>
#include <tc/groups.hpp>

#define NAMED(x) #x, x

void bench(
    std::string group_expr,
    const tc::Group &group,
    const std::vector<tc::Gen> &gens,
    const tc::Coset bound = tc::UNBOUNDED
) {
    std::clock_t s = std::clock();
    tc::Cosets cosets = tc::solve(group, gens, bound);
    std::clock_t e = std::clock();

    auto time = (double) (e - s) / CLOCKS_PER_SEC;
    tc::Coset order = cosets.size();
    auto cos_s = (size_t) (order / time);

    bool complete = cosets.complete;

    std::string name = fmt::format("{}/{}", group_expr, gens);
    std::string row = fmt::format(
        "{:>24},{:>10},{:>6},{:>8.3f}s,{:>10L}",
        name, order, complete, time, cos_s
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

    fmt::print("{:>24},{:>10},{:>6},{:>9},{:>10}\n", "NAME", "ORDER", "COMPL", "TIME", "COS/S");

    bench(NAMED(H(2)), {});
    bench(NAMED(H(3)), {});
    bench(NAMED(H(4)), {});
    
    bench(NAMED(T(100)), {});
    bench(NAMED(T(500)), {});
    bench(NAMED(T(1000)), {});

    bench(NAMED(E(6)), {});
    bench(NAMED(E(7)), {});

    bench(NAMED(B(6)), {});
    bench(NAMED(B(7)), {});
    bench(NAMED(B(8)), {});

    auto g = tc::group::A(4);
    g.set(tc::Rel{0, 3, 3});
    bench("~A(3)", g, {}, 4385964);
    bench("~I(1)", sch(tc::FREE), {}, 4385964);

    return EXIT_SUCCESS;
}
