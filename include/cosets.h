#pragma once

#include "groups.h"
#include <vector>

namespace tc {
    struct Cosets {
        int ngens;
        std::vector<int> data;
        int len;

        Cosets(int ngens, const std::vector<int> &data);

        void add_row();

        void put(int coset, int gen, int target);

        void put(int idx, int target);

        [[nodiscard]] int get(int coset, int gen) const;

        [[nodiscard]] int get(int idx) const;
    };

    struct RelTable {
        Mult mult;

        std::vector<int *> lst_ptr;
        std::vector<int> gen;

        explicit RelTable(tc::Mult m);

        int add_row();
    };
}