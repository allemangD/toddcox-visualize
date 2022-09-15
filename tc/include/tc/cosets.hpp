#pragma once

#include <cstdlib>

#include "cosets.hpp"

namespace tc {
    using Coset = uint32_t;
    using Gen = uint8_t;
    using Mult = uint16_t;

    using Rel = std::tuple<Gen, Gen, Mult>;

    struct Cosets {
        const Coset UNSET = Coset(-1);

        Gen ngens;
        std::vector<int> data;
        Path path;

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


        [[nodiscard]] size_t size() const {
            return path.size();
        }
    };
}