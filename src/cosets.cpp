#include "cosets.h"

namespace tc {
    Cosets::Cosets(int ngens, const std::vector<int> &data) : ngens(ngens), data(data) {
        len = data.size() / ngens;
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

    RelTable::RelTable(tc::Mult m) : mult(m) {
    }

    int RelTable::add_row() {
        int idx = lst_ptr.size();
        lst_ptr.push_back(nullptr);
        gen.push_back(-1);
        return idx;
    }
}