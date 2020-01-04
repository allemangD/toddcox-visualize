#pragma once

#include "cosets.h"

namespace tc {
    Cosets solve(const Group &g, const std::vector<int> &sub_gens = {});
}
