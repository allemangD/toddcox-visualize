#pragma once

#include "groups.h"
#include <vector>

namespace tc {
    struct Action {
        int coset = -1;
        int gen = -1;
        int target = -1;
    };

    struct Cosets {
        int ngens;
        std::vector<int> data;
        std::vector<Action> path;
        int len;

        explicit Cosets(int ngens);

        void add_row();

        void put(int coset, int gen, int target);

        void put(int idx, int target);

        [[nodiscard]] int get(int coset, int gen) const;

        [[nodiscard]] int get(int idx) const;

        [[nodiscard]] int size() const;
    };
}