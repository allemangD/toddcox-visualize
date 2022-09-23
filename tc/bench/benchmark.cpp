#include <ctime>
#include <string>
#include <vector>

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <tc/core.hpp>
#include <tc/groups.hpp>

void bench(
    const std::string &group_expr,
    const std::string &symbol,
    const std::vector<tc::Gen> &gens,
    const tc::Coset bound = tc::UNBOUNDED
) {
    tc::Group group = tc::coxeter(symbol);
    
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

    fmt::print("{:>24},{:>10},{:>6},{:>9},{:>10}\n", "NAME", "ORDER", "COMPL", "TIME", "COS/S");

    bench("H2", "5", {});
    bench("H3", "5 3", {});
    bench("H4", "5 3 3", {});
    
    bench("T100", "100 2 100", {});
    bench("T500", "500 2 500", {});
    bench("T1000", "1000 2 1000", {});

    bench("E6", "3 * [2 2 1]", {});
    bench("E7", "3 * [3 2 1]", {});
//    bench("E8", "3 * [4 2 1]", {});  // too big

    bench("B6", "4 3 * 4", {});
    bench("B7", "4 3 * 5", {});
    bench("B8", "4 3 * 6", {});

    bench("~A3", "{3 * 4}", {}, 5000000);
    bench("~A4", "{3 * 5}", {}, 5000000);
    bench("~I1", "{- * 5}", {}, 5000000);

    return EXIT_SUCCESS;
}
