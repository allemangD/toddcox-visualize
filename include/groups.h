#pragma once

#include <array>
#include <vector>

namespace tc {
    struct Mult {
        std::array<int, 2> gens;
        int mult;
    };

    struct Group {
        int ngens;
        std::vector<std::vector<int>> _mults;

        explicit Group(int ngens, const std::vector<Mult> &rels = {});

        void setmult(Mult m);

        [[nodiscard]] std::vector<Mult> get_mults() const;

        [[nodiscard]] Group product(const Group &other) const;

        [[nodiscard]] Group power(int p) const;
    };

    Group operator*(const Group &g, const Group &h);

    Group operator^(const Group &g, const int &p);

    Group schlafli(const std::vector<int> &mults);

    namespace group {
        Group A(int dim);

        Group B(int dim);

        Group D(int dim);

        Group E(int dim);

        Group F4();

        Group G2();

        Group H(int dim);

        Group I2(int n);

        Group T(int n, int m);

        Group T(int n);
    }
}