#pragma once

#include <cstdlib>

#include "util.hpp"

namespace tc {
    struct Cosets {
        Gen ngens;
        std::vector<int> data;
        Path path;
        bool complete = false;

        Cosets(const Cosets &) = default;

        explicit Cosets(Gen ngens)
            : ngens(ngens) {
        }

        void add_row() {
            data.resize(data.size() + ngens, UNSET);
            path.add_row();
        }

        void put(Coset coset, Gen gen, Coset target) {
            data[coset * ngens + gen] = target;
            data[target * ngens + gen] = coset;

            if (path.get(target).from_idx == UNSET) {
                path.put(coset, gen, target);
            }
        }

        void put(size_t idx, Coset target) {
            Coset coset = idx / ngens;
            Gen gen = idx % ngens;

            data[idx] = target;
            data[target * ngens + gen] = coset;

            if (path.get(target).from_idx == UNSET) {
                path.put(coset, gen, target);
            }
        }

        [[nodiscard]] Coset get(Coset coset, Gen gen) const {
            return data[coset * ngens + gen];
        }

        [[nodiscard]] Coset get(size_t idx) const {
            return data[idx];
        }

        [[nodiscard]] bool isset(Coset coset, Gen gen) const {
            return get(coset, gen) != UNSET;
        }

        [[nodiscard]] bool isset(size_t idx) const {
            return get(idx) != UNSET;
        }


        [[nodiscard]] tc::Coset size() const {
            return path.size();
        }
    };
}