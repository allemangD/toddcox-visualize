#include <vector>
#include <stack>
#include <string>

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <peglib.h>

struct Op {
    enum Code {
        LINK,
        PUSH,
        POP,
        LOOP,
    };

    Code code: 4;
    unsigned int value: 12;

    explicit Op(Code code, unsigned int value = 0)
        : code(code), value(value) {}
};

template<>
struct fmt::formatter<Op> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const Op &op, FormatContext &ctx) {
        switch (op.code) {
            case Op::LINK:
                return fmt::format_to(ctx.out(), "link({})",
                                      (unsigned int) op.value);
            case Op::PUSH:
                return fmt::format_to(ctx.out(), "push()");
            case Op::POP:
                return fmt::format_to(ctx.out(), "pop()");
            case Op::LOOP:
                return fmt::format_to(ctx.out(), "loop()");
            default:
                return fmt::format_to(ctx.out(), "[{}]({})",
                                      (unsigned int) op.code,
                                      (unsigned int) op.value);
        }
    }
};

struct Factor {
    unsigned int mode;
    std::vector<unsigned int> orders;
};

struct codegen {
    std::vector<Op> ops;

    void link(unsigned int order) {
        ops.emplace_back(Op::LINK, order);
    }

    void push() {
        ops.emplace_back(Op::PUSH);
    }

    void pop() {
        ops.emplace_back(Op::POP);
    }

    void loop() {
        ops.emplace_back(Op::LOOP);
    }

    template<typename It>
    void insert(It begin, It end) {
        std::copy(begin, end, std::back_inserter(ops));
    }

    void insert(const codegen &o) {
        insert(o.ops.begin(), o.ops.end());
    }
};

peg::parser build_parser() {
    const auto grammar = R"(
        root    <- term+
        term    <- product / op
        op      <- block / link
        block   <- '(' root ')' / '{' root '}' / '[' root ']'
        link    <- int
        product <- op '*' factor
        factor  <- int / '{' int+ '}' / '[' int+ ']'
        int     <- < [0-9]+ >

        %whitespace <- [ \t\n\r]*
    )";

    peg::parser parser;
    parser.set_logger([](size_t line, size_t col, const std::string &msg, const std::string &rule) {
        fmt::print(stderr, "{}:{} [{}] {}\n", line, col, rule, msg);
    });
    auto ok = parser.load_grammar(grammar);
    assert(ok);

    parser["int"] = [](const peg::SemanticValues &vs) -> std::any {
        return vs.token_to_number<unsigned int>();
    };

    parser["link"] = [](const peg::SemanticValues &vs) -> std::any {
        codegen cg;

        auto order = std::any_cast<unsigned int>(vs[0]);
        cg.link(order);

        return cg;
    };

    parser["root"] = [](const peg::SemanticValues &vs) -> std::any {
        codegen cg;

        for (const auto &sub: vs) {
            cg.insert(std::any_cast<codegen>(sub));
        }

        return cg;
    };

    parser["block"] = [](const peg::SemanticValues &vs) -> std::any {
        if (vs.choice() == 0) return vs[0];

        codegen cg;

        cg.push();
        cg.insert(std::any_cast<codegen>(vs[0]));
        if (vs.choice() == 1) cg.loop();
        cg.pop();

        return cg;
    };

    parser["factor"] = [](const peg::SemanticValues &vs) -> std::any {
        return Factor{
            (unsigned int) vs.choice(),
            vs.transform<unsigned int>(),
        };
    };

    parser["product"] = [](const peg::SemanticValues &vs) -> std::any {
        auto sub = std::any_cast<codegen>(vs[0]);
        auto fac = std::any_cast<Factor>(vs[1]);

        codegen cg;

        for (const auto &order: fac.orders) {
            if (fac.mode == 0) {
                for (unsigned int i = 0; i < order; ++i) {
                    cg.insert(sub);
                }
            } else {
                cg.push();
                for (unsigned int i = 0; i < order; ++i) {
                    cg.insert(sub);
                }
                if (fac.mode == 1) cg.loop();
                cg.pop();
            }
        }

        return cg;
    };

    return parser;
}

std::vector<Op> compile(const std::string &source) {
    static peg::parser parser = build_parser();

    std::any val;
    codegen cg;

    bool success = parser.parse(source, cg);
    assert(success);

    return cg.ops;
}

struct Graph {
    size_t rank{};
    std::vector<std::tuple<size_t, size_t, unsigned int>> edges{};
};

Graph eval(const std::vector<Op> &ops) {
    std::vector<std::stack<size_t>> stacks(1);

    Graph g;
    stacks.back().emplace(g.rank++);

    for (const auto &op: ops) {
        switch (op.code) {
            case Op::LINK: {
                auto order = op.value;

                auto top = stacks.back().top();
                auto curr = g.rank++;

                stacks.back().emplace(curr);
                g.edges.emplace_back(top, curr, order);

                break;
            }
            case Op::PUSH: {
                stacks.emplace_back();

                auto ptop = stacks[stacks.size() - 2].top();
                stacks.back().emplace(ptop);

                break;
            }
            case Op::POP: {
                stacks.pop_back();

                break;
            }
            case Op::LOOP: {
                g.rank--;

                auto ptop = stacks[stacks.size() - 2].top();
                auto &[top, _, order] = g.edges.back();

                stacks.back().pop();
                g.edges.pop_back();

                g.edges.emplace_back(top, ptop, order);

                break;
            }
            default:
                throw std::runtime_error("Invalid opcode");
        }
    }

    return g;
}

int main() {
    std::vector<std::string> sources = {
        "5 3 3",
        "5 (3 3)",
        "[5 3 3]",
        "[4 3 [3 5] 3]",
        "{3 4 3 5}",
        "3 {3 3 [4] 3} 5",
        "5 * 3",
        "5 * [3]",
        "5 * {2 3}",
        "5 * [3 2]",
        "(5 2) * [3 2]",
        "4 [3 * [2 3]] 5",
        "{3 * 3} [4] [5]",
    };

    for (const auto &source: sources) {
        fmt::print("source: '{}'\n", source);

        auto ops = compile(source);
        for (const auto &op: ops) {
            fmt::print("-  {}\n", op);
        }
        auto g = eval(ops);
        fmt::print("rank: {}\n", g.rank);
        for (const auto &edge: g.edges) {
            fmt::print("- {}\n", edge);
        }
        fmt::print("=======================================\n");
    }

    return 0;
}
