#include <tc/core.hpp>

namespace tc {
    SubGroup Group::subgroup(const std::vector<int> &gens) const {
        return SubGroup(*this, gens);
    }
}
