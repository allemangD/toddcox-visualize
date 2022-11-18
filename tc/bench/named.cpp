#include <string>
#include <vector>

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <tc/core.hpp>

template<typename G>
void show(G const &g) {  // todo add formatter for groups, cosets.
    fmt::print("  | ");
    for (const auto &v: g.gens()) {
        fmt::print("{} ", v);
    }
    fmt::print("\n");

    for (const auto &u: g.gens()) {
        fmt::print("{} | ", u);
        for (const auto &v: g.gens()) {
            fmt::print("{} ", g.get(u, v));
        }
        fmt::print("\n");
    }
}

int main() {
    tc::Group<char> group(4, {'r', 'g', 'b', 'y'});

    group.set('r', 'g', 5);
    group.set('g', 'b', 4);
    group.set('b', 'y', 3);

    show(group);

    auto sub = group.sub({'r', 'g', 'y'});
    show(sub);

    auto res = sub.solve({});
    fmt::print("res order: {}\n", res.order());

    auto cos = sub.solve({'r', 'y'});

    fmt::print("order: {}\n", cos.order());

//    tc::Group<u_int8_t> group(4, {0, 1, 2, 3});
//
//    group.set(0, 1, 5);
//    group.set(1, 2, 4);
//    group.set(2, 3, 3);
//
//    show(group);
//
//    auto sub = group.sub({3, 2, 0, 1});
//    show(sub);
}
