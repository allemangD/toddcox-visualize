#include "cosets.h"

namespace tc {
    Cosets::Cosets(int ngens) : ngens(ngens), len(0) {
    }

    void Cosets::add_row() {
        len++;
        data.resize(data.size() + ngens, -1);
    }

    void Cosets::put(int coset, int gen, int target) {
        data[coset * ngens + gen] = target;
        data[target * ngens + gen] = coset;
    }

    void Cosets::put(int idx, int target) {
        int coset = idx / ngens;
        int gen = idx % ngens;
        data[idx] = target;
        data[target * ngens + gen] = coset;
    }

    int Cosets::get(int coset, int gen) const {
        return data[coset * ngens + gen];
    }

    int Cosets::get(int idx) const {
        return data[idx];
    }

}