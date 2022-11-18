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
    const std::vector<size_t> &gens,
    const size_t bound = SIZE_MAX
) {
    tc::Group<> group = tc::coxeter(symbol);
    
    std::clock_t s = std::clock();
    tc::Cosets<> cosets = group.solve(gens, bound);
    std::clock_t e = std::clock();

    auto time = (double) (e - s) / CLOCKS_PER_SEC;
    size_t order = cosets.order();
    auto cos_s = (size_t) (order / time);

    bool complete = cosets.complete();

    std::string name = fmt::format("{}/{}", group_expr, gens);
    std::string row = fmt::format(
        "{:>24},{:>10},{:>6},{:>8.3f}s,{:>10L}",
        name, order, complete, time, cos_s
    );
    fmt::print("{}\n", row);
}

int main(int argc, char *argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);

    fmt::print("{:>24},{:>10},{:>6},{:>9},{:>10}\n", "NAME", "ORDER", "COMPL", "TIME", "COS/S");

    // Finite Groups
    
    // A_n: 3 * `n-1`           ; n >= 1
    bench("A_5", "3 * 4", {});
    bench("A_6", "3 * 5", {});
    bench("A_7", "3 * 6", {});
    bench("A_8", "3 * 7", {});
    // B_n: 4 3 * `n-2`         ; n >= 2
    bench("B_5", "4 3 * 3", {});
    bench("B_6", "4 3 * 4", {});
    bench("B_7", "4 3 * 5", {});
    bench("B_8", "4 3 * 6", {});
    // D_n: 3 * [1 1 `n-3`]     ; n >= 4
    bench("D_5", "3 * [1 1 2]", {});
    bench("D_6", "3 * [1 1 3]", {});
    bench("D_7", "3 * [1 1 4]", {});
    bench("D_8", "3 * [1 1 5]", {});
    // E_n: 3 * [1 2 `n-4`]     ; n >= 6
    bench("E_6", "3 * [1 2 2]", {});
    bench("E_7", "3 * [1 2 3]", {});
//    bench("E_8", "3 * [1 2 4]", {}); // too big
    // H_n: 5 3 * `n-2`         ; n >= 2
    bench("H_3", "5 3 * 0", {});
    bench("H_4", "5 3 * 1", {});
    bench("H_5", "5 3 * 2", {});
    // grid: `p` `q`            ; 2(p+q) > pq
    // triangle: `p` `q` `r`    ; 1/p + 1/q + 1/r > 1

    // Special Finite Groups
    bench("F_4", "3 4 3", {});
    bench("G_2", "6", {});
    // I_2(p): `p`              ; p >= 2
    bench("I_2(100)", "100", {});
    bench("I_2(1000)", "1000", {});
    // "Torus": `p` 2 `q`       ; p, q >= 2
    bench("T(100)", "100 2 100", {});
    bench("T(1000)", "1000 2 1000", {});

    // Affine Groups
    
    // ~A_n: {3 * `n+1`}
    bench("~A_5", "{3 * 6}", {}, 10'000'000);
    bench("~A_6", "{3 * 7}", {}, 10'000'000);
    bench("~A_7", "{3 * 8}", {}, 10'000'000);
    bench("~A_8", "{3 * 9}", {}, 10'000'000);
    // ~B_n: 4 3 * `n-3` 3 * [1 1]
    bench("~B_5", "4 3 * 2 3 * [1 1]", {}, 10'000'000);
    bench("~B_6", "4 3 * 3 3 * [1 1]", {}, 10'000'000);
    bench("~B_7", "4 3 * 4 3 * [1 1]", {}, 10'000'000);
    bench("~B_8", "4 3 * 5 3 * [1 1]", {}, 10'000'000);
    // ~B_n: 4 3 * `n-2` 4
    bench("~C_5", "4 3 * 3 4", {}, 10'000'000);
    bench("~C_6", "4 3 * 4 4", {}, 10'000'000);
    bench("~C_7", "4 3 * 5 4", {}, 10'000'000);
    bench("~C_8", "4 3 * 6 4", {}, 10'000'000);
    // ~D_n: 3 * [1 1] 3 * `n-4` 3 * [1 1]
    bench("~D_5", "3 * [1 1] 3 * 1 3 * [1 1]", {}, 10'000'000);
    bench("~D_6", "3 * [1 1] 3 * 2 3 * [1 1]", {}, 10'000'000);
    bench("~D_7", "3 * [1 1] 3 * 3 3 * [1 1]", {}, 10'000'000);
    bench("~D_8", "3 * [1 1] 3 * 4 3 * [1 1]", {}, 10'000'000);
    // grid: `p` `q`            ; 2(p+q) = pq
    // triangle: `p` `q` `r`    ; 1/p + 1/q + 1/r = 1
    
    // Special Affine Groups
    bench("~I_1", "-", {}, 10'000'000);
    bench("~E_6", "3 * [2 2 2]", {}, 10'000'000);
    bench("~E_7", "3 * [1 3 3]", {}, 10'000'000);
    bench("~E_8", "3 * [1 2 5]", {}, 10'000'000);
//    bench("E_9",  "3 * [1 2 5]", {}, 10'000'000);  // ~E_8 == E_9
    bench("~F_4", "3 4 3 3", {}, 10'000'000);
    bench("~G_2", "6 3", {}, 10'000'000);

    // Hyperbolic Groups
    // grid: `p` `q`            ; 2(p+q) < pq
    // triangle: `p` `q` `r`    ; 1/p + 1/q + 1/r < 1
    
    // Special Hyperbolic Groups
    bench("-BH_3", "4 3 5", {}, 10'000'000);
    bench("-K_3", "5 3 5", {}, 10'000'000);
    bench("-J_3", "3 5 3", {}, 10'000'000);
//    bench("~H_3", "3 5 3", {}, 10'000'000);  // -J_3 == ~H_3
    bench("-DH_3", "5 3 * [1 1]", {}, 10'000'000);
    bench("^AB_3", "{3 3 3 4}", {}, 10'000'000);
    bench("^AH_3", "{3 3 3 5}", {}, 10'000'000);
    bench("^BB_3", "{3 4 3 4}", {}, 10'000'000);
    bench("^BH_3", "{3 4 3 5}", {}, 10'000'000);
    bench("^HH_3", "{3 5 3 5}", {}, 10'000'000);
    bench("-H_4", "5 3 3 3", {}, 10'000'000);
//    bench("~H_4", "5 3 3 3", {}, 10'000'000);  // -H_4 == ~H_4 == H_5 
//    bench("H_5", "5 3 3 3", {}, 10'000'000);
    bench("-BH_4", "4 3 3 5", {}, 10'000'000);
    bench("-K_4", "5 3 3 5", {}, 10'000'000);
    bench("-DH_4", "5 3 3 * [1 1]", {}, 10'000'000);
    bench("^AF_4", "{3 3 3 3 4}", {}, 10'000'000);
    
    return EXIT_SUCCESS;
}
