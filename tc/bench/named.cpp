#include <string>
#include <vector>

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <tc/core.hpp>

using namespace std::string_literals;

template<typename Gen_>
struct fmt::formatter<tc::Group<Gen_>> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }

    template<typename FormatContext>
    constexpr auto format(tc::Group<Gen_> const &g, FormatContext &ctx) {
        auto gens = g.gens();

        fmt::format_to(ctx.out(), "  | ");
        for (const auto &gen: gens) {
            fmt::format_to(ctx.out(), "{} ", gen);
        }
        fmt::format_to(ctx.out(), "\n");

        for (int i = 0; i < gens.size(); ++i) {
            auto u = gens[i];

            fmt::format_to(ctx.out(), "{} | ", u);

            for (int j = 0; j < gens.size(); ++j) {
                auto v = gens[j];

                if (i <= j) {
                    fmt::format_to(ctx.out(), "{} ", g.get(u, v));
                } else {
                    fmt::format_to(ctx.out(), "  ");
                }
            }

            fmt::format_to(ctx.out(), "\n");
        }

        return ctx.out();
    }
};

template<typename Gen_>
struct fmt::formatter<tc::Cosets<Gen_>> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }

    template<typename FormatContext>
    constexpr auto format(tc::Cosets<Gen_> const &c, FormatContext &ctx) {
        auto gens = c.gens();

        auto width = (size_t) (std::log10(c.order() + 1));

        fmt::format_to(ctx.out(), "{:>{}} | ", "-", width);
        for (const auto &gen: gens) {
            fmt::format_to(ctx.out(), "{:>{}} ", gen, width);
        }
        fmt::format_to(ctx.out(), "\n");

        for (size_t cos = 0; cos < c.order(); ++cos) {
            fmt::format_to(ctx.out(), "{:>{}} | ", cos, width);

            for (const auto &gen: gens) {
                auto target = c.get(cos, gen);

                fmt::format_to(ctx.out(), "{:>{}} ", target, width);
            }

            fmt::format_to(ctx.out(), "\n");
        }

        return ctx.out();
    }
};

int main() {
    tc::Group<char> group(4, {'r', 'g', 'b', 'y'});

    group.set('r', 'g', 5);
    group.set('g', 'b', 3);
    group.set('b', 'y', 3);

    fmt::print("{}\n", group);

//    auto res = group.sub({'r', 'g', 'b', 'y'}).solve();
    auto res = group.sub({'r', 'g'}).solve();
    fmt::print("rank: {}, order: {}\n{}\n",
               res.rank(), res.order(), res);

//    tc::Path<> path(res);
//    auto gens = group.gens();
//    std::vector<std::string> words(path.order());
//    path.walk(
//        "-"s,
//        [&](const std::string &cos, size_t gen) -> std::string {
//            return cos + gens[gen];
//        },
//        words.begin());

    tc::Path<char> typed(res);
    std::vector<std::string> words(typed.order());
    typed.walk("-"s, std::plus<>(), words.begin());

    fmt::print("words: {}\n", words);
    fmt::print("size: {}\n", words.size());

//    path.walk(
//        words.begin(),
//        "-",
//        [&gens](const std::string &cos, size_t gen) {
//            return cos + gens[gen];
//        }
//    );
//    for (const auto &word: words) {
//        fmt::print("'{}'\n", word);
//    }

//    auto sub = group.sub({'r', 'g', 'y'});
//    fmt::print("{}\n", sub);
//
//    auto res = sub.solve({});
//    fmt::print("res order: {}\n", res.order());
//
//    auto cos = sub.solve({'r', 'y'});

//    fmt::print("order: {}\n", cos.order());
//
//    fmt::print("{}\n", cos);

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
